#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "shell.h"

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
                          &infop, WNOHANG|WCONTINUED|WEXITED|WSTOPPED|WUNTRACED);
            if(code != -1) {
                update_job_counters(i, infop.si_code);
            }
        }
        if(jobs.arr[i].cnt_process > 0) {
            enum jstatus prevstat = jobs.arr[i].stat;
            if(jobs.arr[i].cnt_ended == jobs.arr[i].cnt_process && infop.si_code != CLD_KILLED) {
                jobs.arr[i].stat = DONE;
                if(jobs.arr[i].instoplist) {
                    stoplist_del(i);
                }
            } else if(jobs.arr[i].cnt_running == 0 && jobs.arr[i].cnt_stopped > 0) {
                jobs.arr[i].stat = STOPPED;
                if(!jobs.arr[i].instoplist) {
                    stoplist_add(i);
                }
            } else if(jobs.arr[i].cnt_ended == jobs.arr[i].cnt_process && infop.si_code == CLD_KILLED) {
                jobs.arr[i].stat = TERMINATED;  
                if(jobs.arr[i].instoplist) {
                    stoplist_del(i);
                } 
            } else if(jobs.arr[i].cnt_running > 0) {
                jobs.arr[i].stat = RUNNING;
                if(!jobs.arr[i].instoplist) {
                    stoplist_add(i);
                }
                //stoplist_del(i);
            }
            if(jobs.arr[i].stat != prevstat) {
                changed = 1;
            }
        }
    }
    int newlast = last_job_id();
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
    if(tcsetpgrp(0, jobs.arr[job_id].lidpid) == -1) {
        perror("unable to set processs to fg");
    }
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
    if(jobs.arr[job_id].stat == RUNNING) {
        job_to_fg(job_id, 0);
    } else {
        job_to_fg(job_id, 1);
    }
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
    jobs.arr[job_id].cnt_running = jobs.arr[job_id].cnt_stopped;
    jobs.arr[job_id].cnt_stopped = 0;
    char sign = ' ';
    if(job_id == jobs.plus_id) {
        sign = '+';
    } else if(job_id == jobs.mins_id) {
        sign = '-';
    }
    printf("[%d]%c %s &\n", job_id, sign, jobs.arr[job_id].cmdline);
    
    sigsend(P_PGID, jobs.arr[job_id].lidpid, SIGCONT);
}