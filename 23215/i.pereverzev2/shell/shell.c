#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include "shell.h"
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;
char emptyline;


typedef struct jobs_s {
    struct job *arr;
    int arsz;
    int last_id;
    int plus_id;
    int mins_id;
} jobsinfo;

jobsinfo jobs;
struct termios shell_tattr;

void stoplist_add(int job_id) 
{
    // add only to the last element
    jobs.arr[jobs.arr[0].prevjob].nextjob = job_id;
    jobs.arr[job_id].prevjob = jobs.arr[0].prevjob;
    jobs.arr[0].prevjob = job_id;
    jobs.arr[job_id].nextjob = 0;
    // not forget about plus and minus shift
    jobs.mins_id = jobs.plus_id;
    jobs.plus_id = job_id;
    jobs.arr[job_id].instoplist = 1;
}

void stoplist_del(int job_id)
{
    // delete any element specified by job_id
    if(jobs.plus_id == job_id) {
        jobs.plus_id = jobs.mins_id;
        jobs.mins_id = jobs.arr[jobs.mins_id].prevjob;
    } else if(jobs.mins_id == job_id) {
        jobs.mins_id = jobs.arr[jobs.mins_id].prevjob;
    }
    jobs.arr[jobs.arr[job_id].nextjob].prevjob = jobs.arr[job_id].prevjob;
    jobs.arr[jobs.arr[job_id].prevjob].nextjob = jobs.arr[job_id].nextjob;
    jobs.arr[job_id].instoplist = 0;
}

void init_jobs()
{
    jobs.arsz = 2;
    jobs.arr = calloc(jobs.arsz, sizeof(struct job));
    jobs.plus_id = 0;
    jobs.last_id = 0;
    jobs.mins_id = -1;
}

void ensure_joblist_size()
{
    int job_id = jobs.last_id;
    if(jobs.arsz > job_id) {
        return;
    }
    struct job* oldarr = jobs.arr;
    int newsz = jobs.arsz * 2;
    jobs.arr = calloc(newsz, sizeof(struct job));
    memcpy(jobs.arr, oldarr, sizeof(struct job) * (jobs.last_id + 1));
    free(oldarr);
    jobs.arsz = newsz;
}

void clear_jobs()
{
    int i, k;
    for(i = 0; i < jobs.arsz; i++) {
        jobs.arr[i].pgid = 0;
        jobs.arr[i].lidpid = 0;
        jobs.arr[i].fgrnd = 0;
        jobs.arr[i].stat = NONE;
        for(k = 0; k < MAXLINELEN; k++) {
            jobs.arr[i].cmdline[k] = 0;
        }
    }
}

int last_job_id()
{
    int i;
    for(i = jobs.arsz - 1; i >= 1; i--) {
        if(jobs.arr[i].stat != NONE) {
            return i;
        }
    }
    return 0;
}


void update_job_counters(int job_id, int si_code) {
    if(si_code == CLD_STOPPED){
        jobs.arr[job_id].cnt_running--;
        jobs.arr[job_id].cnt_stopped++;
    } else if (si_code == CLD_EXITED || si_code == CLD_KILLED) {
        jobs.arr[job_id].cnt_running--;
        jobs.arr[job_id].cnt_ended++;
    } else if (si_code == CLD_CONTINUED) {
        jobs.arr[job_id].cnt_stopped--;
        jobs.arr[job_id].cnt_running++;
    }
}

int update_jobs()
{
    int changed = 0;
    int i;
    int code;
    siginfo_t infop;
    for(i = 1; i < jobs.arsz; i++) {
        if(jobs.arr[i].stat == NONE) {
            continue;
        }
        int k;
        for(k = 0; k < jobs.arr[i].cnt_process; k++) {
            infop.si_code = -1;
            code = waitid(P_PGID, jobs.arr[i].lidpid,
                          &infop, WNOHANG|WCONTINUED|WEXITED|WSTOPPED);
            if(code != -1) {
                update_job_counters(i, infop.si_code);
            }
        }
        if(jobs.arr[i].cnt_process > 0) {
            enum jstatus prevstat = jobs.arr[i].stat;
            if(jobs.arr[i].cnt_ended == jobs.arr[i].cnt_process && infop.si_code != CLD_KILLED) {
                jobs.arr[i].stat = DONE;
                stoplist_del(i);
            } else if(jobs.arr[i].cnt_running == 0 && jobs.arr[i].cnt_stopped > 0) {
                jobs.arr[i].stat = STOPPED;
                stoplist_add(i);
            } else if(jobs.arr[i].cnt_ended == jobs.arr[i].cnt_process && infop.si_code == CLD_KILLED) {
                jobs.arr[i].stat = TERMINATED;   
                stoplist_del(i);
            } else if(jobs.arr[i].cnt_running > 0) {
                jobs.arr[i].stat = RUNNING;
                stoplist_del(i);
            }
            if(jobs.arr[i].stat != prevstat) {
                changed = 1;
            }
        }
    }
    int newlast = last_job_id();
    printf("newlast is: %d\n", newlast);
    if(newlast == 0) {
        jobs.plus_id = 0;
        jobs.mins_id = -1;
        jobs.last_id = 0;
        clear_jobs();
    } else {
        jobs.last_id = newlast;
    }
    return changed;
}

void print_jobs(int only_ended)
{
    int i;
    for(i = 0; i < jobs.arsz; i++) {
        char sign = 0;
        if(i == jobs.plus_id) {
            sign = '+';
        } else if(i == jobs.mins_id) {
            sign = '-';
        } else {
            sign = ' ';
        }

        if(jobs.arr[i].stat == DONE) {
            fprintf(stderr, "[%d]%c Done\t\t%s\n",
                    i, sign, jobs.arr[i].cmdline);
            jobs.arr[i].stat = NONE;
        } else if(jobs.arr[i].stat == TERMINATED) {
            fprintf(stderr, "[%d]%c Terminated\t\t%s\n",
                    i, sign, jobs.arr[i].cmdline);
            jobs.arr[i].stat = NONE;
        } else if(only_ended == 0) {
            if(jobs.arr[i].stat == STOPPED) {
                fprintf(stderr, "[%d]%c Stopped\t\t%s\n", 
                        i, sign, jobs.arr[i].cmdline);
            } else if(jobs.arr[i].stat == RUNNING) {
                fprintf(stderr, "[%d]%c Running\t\t%s\n", 
                        i, sign, jobs.arr[i].cmdline);
            }
        }
    }
}

int get_job_id(char* spec)
{
    if(spec==NULL || spec[0] != '%' || spec[1] == 0) {
        return -1; // incorrect argument
    }
    if(spec[1] == '+' || spec[1] == '%') {
        return jobs.plus_id;
    }
    if(spec[1] == '-') {
        return jobs.mins_id;
    }
    int id = atoi(spec + 1);
    if(id == 0) {
        return -1; // again incorrect argument
    }
    return id;
}


void job_to_fg(int job_id, int need_cont_sig) {
    siginfo_t infop;
    tcsetpgrp(0, jobs.arr[job_id].lidpid);
    jobs.arr[job_id].fgrnd = 1;
    
    if(need_cont_sig) {
        if(need_cont_sig == 1) {
            if(tcsetattr(0, TCSADRAIN, &(jobs.arr[job_id].jobattr)) == -1) {
                perror("unable to return job's terminal attributes");
            }
            jobs.arr[job_id].stat = RUNNING;
            jobs.arr[job_id].cnt_running += jobs.arr[job_id].cnt_stopped;
            jobs.arr[job_id].cnt_stopped = 0;
        }
        sigsend(P_PGID, jobs.arr[job_id].lidpid, SIGCONT);
    }

    while(jobs.arr[job_id].cnt_running > 0) {
        infop.si_code = -1;
        pid_t code = waitid(P_PGID, jobs.arr[job_id].lidpid, &infop,
                        WSTOPPED | WEXITED);
        if(code == -1) {
            perror("unable to wait termination of one of job processes");
            break;
        }
        update_job_counters(job_id, infop.si_code);
    }
    
    if(tcgetattr(0, &(jobs.arr[job_id].jobattr)) == -1) {
        perror("unable to get terminal attributes after job");
    }
    if(tcsetpgrp(0, getpgrp()) == -1) {
        perror("unable to return shell to fg");
    }
    if(tcsetattr(0, TCSADRAIN, &shell_tattr) == -1) {
        perror("unable to return shell terminal attributes");
    }

    if(jobs.arr[job_id].cnt_stopped > 0 && jobs.arr[job_id].cnt_running == 0){
        jobs.arr[job_id].stat = STOPPED;
        jobs.arr[job_id].fgrnd = 0;
        fprintf(stderr, "\n[%d]+ job Stopped\t\t%s\n",job_id, 
                jobs.arr[job_id].cmdline);
        if(jobs.arr[job_id].instoplist == 0) {
            stoplist_add(job_id);            
        }
    } else if (jobs.arr[job_id].cnt_ended == jobs.arr[job_id].cnt_process) {
        jobs.arr[job_id].stat = NONE;
        if(jobs.arr[job_id].instoplist == 1) {
            stoplist_del(job_id);            
        }
    }
}

void fg(char* arg) {
    int job_id = jobs.plus_id;
    if(arg != NULL) {
        // job is specified
        job_id = get_job_id(arg);
        if(job_id == -1) {
            fprintf(stderr, "incorrect argument of fg\n");
            return;
        }
    }
    job_to_fg(job_id, 1);
}

void bg(char* arg){
    int job_id = jobs.plus_id;
    if(arg != NULL) {
        // job is specified
        job_id = get_job_id(arg);
        if(job_id == -1) {
            fprintf(stderr, "incorrect argument of fg\n");
            return;
        }
    }
    jobs.arr[job_id].stat = RUNNING;
    printf("bg, stopped %d\n", jobs.arr[job_id].cnt_stopped);
    jobs.arr[job_id].cnt_running = jobs.arr[job_id].cnt_stopped;
    jobs.arr[job_id].cnt_stopped = 0;
    sigsend(P_PGID, jobs.arr[job_id].lidpid, SIGCONT);
}

int fake_pipe(int* arr)
{
    static int pcnt = 0;
    pcnt++;
    arr[0] = pcnt;
    pcnt++;
    arr[1] = pcnt;
    return 0;
}

int main()
{
    register int i;
    char line[MAXLINELEN];      /*  allow large command lines  */
    int ncmds;
    char prompt[] = "sh$ ";      /* shell prompt */    
    tcgetattr(0, &shell_tattr);
    init_jobs();
    /* PLACE SIGNAL CODE HERE */

    sigset(SIGINT, SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);
    sigset(SIGTTOU, SIG_IGN);
    sigset(SIGTTIN, SIG_IGN);
    sigset(SIGTSTP, SIG_IGN);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /*until eof  */
        int updcode = update_jobs();
        if ((ncmds = parseline(line)) <= 0) {
            if(updcode) {
                print_jobs(1);
            }
            continue;   /* read next line */
        }
#ifdef DEBUG
{
    int i, j;
    for (i = 0; i < ncmds; i++) {
        for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
            fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
                    i, j, cmds[i].cmdargs[j]);
        fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
    }
}
#endif
        int pipe_fds_cur[2] = {0};
        int pipe_fds_prev[2] = {0};
        for (i = 0; i < ncmds; i++) {
            
            // parsing of jobs, fg and bg
            if(!strcmp(cmds[i].cmdargs[0], "jobs")) {
                update_jobs();
                print_jobs(0);
                continue;
            } else if(!strcmp(cmds[i].cmdargs[0], "fg")) {
                fg(cmds[i].cmdargs[1]);
                continue;
            } else if(!strcmp(cmds[i].cmdargs[0], "bg")) {
                bg(cmds[i].cmdargs[1]);
                continue;
            }
            
            /*  FORK AND EXECUTE  */
            pipe_fds_prev[0] = pipe_fds_cur[0];
            pipe_fds_prev[1] = pipe_fds_cur[1];
            if(cmds[i].cmdflag & OUTPIP) {    
                if(pipe(pipe_fds_cur) == -1) {
                    perror("unable to create pipe between processes");
                }
            }
            int chpid = fork();
            if (chpid == 0) {
                // child
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);                
                sigset(SIGTTOU, SIG_DFL);
                sigset(SIGTTIN, SIG_DFL);
                if(cmds[i].cmdflag & INPIP){
                    if(setpgid(0, jobs.arr[jobs.last_id].lidpid) == -1) {
                        perror("unable to place process in his job group from child");
                        return 1;
                    }
                } else {
                    if(setpgid(0, 0) == -1) {
                        perror("unable to place process in his job group from child");
                        return 1;
                    }
                }
                
                if (bkgrnd == 0 && (!(cmds[i].cmdflag & INPIP))) {
                    tcsetpgrp(0, getpgrp());
                }
                if (infile) {
                    int inputfd = open(infile, O_RDONLY);
                    if (inputfd == -1) {
                        perror("unable to open file for reading");
                        return 2;
                    }
                    dup2(inputfd, 0);
                }
                if (outfile) {
                    int outputfd = open(outfile, O_WRONLY | O_CREAT, 
                                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if (outputfd == -1) {
                        perror("unable to open file for writing");
                        return 3;
                    }
                    dup2(outputfd, 1);
                }
                if (appfile) {
                    int appfd = open(appfile, O_WRONLY | O_APPEND);
                    if (appfd == -1) {
                        perror("unable to open file for appending");
                        return 4;
                    }
                    dup2(appfd, 1);
                }
                if (cmds[i].cmdflag & INPIP) {
                    dup2(pipe_fds_prev[1], 0);
                }
                if (cmds[i].cmdflag & OUTPIP) {
                    dup2(pipe_fds_cur[0], 1);
                }
                
                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                fprintf(stderr, "unable to execute %s\n", cmds[i].cmdargs[0]);
                return 5;
            } else if (chpid == -1) {
                // fork error
                perror("unable to fork, out of resources to create process");
            } else {
                // parent
                if(!(cmds[i].cmdflag & INPIP)) {
                    // ensure that we are on first process in pipeline
                    jobs.last_id++;
                    ensure_joblist_size();
                    jobs.arr[jobs.last_id].lidpid = chpid;
                    jobs.arr[jobs.last_id].stat = RUNNING;
                    jobs.arr[jobs.last_id].cnt_stopped = 0;
                    int k = 0;
                    jobs.arr[jobs.last_id].cmdline[0] = 0;
                    while(1) {
                        // store launch command to job
                        strcat(jobs.arr[jobs.last_id].cmdline, cmds[i].cmdargs[k]);
                        if(cmds[i].cmdargs[k+1]) {
                            strcat(jobs.arr[jobs.last_id].cmdline, " ");
                        } else {
                            break;
                        }
                        k++;
                    }
                    printf("[%d] %d\n", jobs.last_id, chpid);                
                }
                jobs.arr[jobs.last_id].cnt_process++;
                jobs.arr[jobs.last_id].cnt_running++;
                if(setpgid(chpid, jobs.arr[jobs.last_id].lidpid) == -1) {
                    perror("unable to place process in his job group from parent");
                    return 1;
                }
                if (bkgrnd == 0 && !(cmds[i].cmdflag & OUTPIP)) {
                    // launch job in foreground (if we are on last process in pipeline)
                    job_to_fg(jobs.last_id, 2);
                }
            }
        }
    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */

