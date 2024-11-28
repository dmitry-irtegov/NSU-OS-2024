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

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

jobsinfo jobs;
struct termios shell_tattr;


int jobcontrol(char **cmdargs)
{
    if(!strcmp(cmdargs[0], "jobs")) {
        update_jobs();
        print_jobs(0);
        return 1;
    } else if(!strcmp(cmdargs[0], "fg")) {
        fg(cmdargs[1]);
        return 1;
    } else if(!strcmp(cmdargs[0], "bg")) {
        bg(cmdargs[1]);
        return 1;
    }
    return 0;
}

void set_files_io()
{
    if (infile) {
        int inputfd = open(infile, O_RDONLY);
        if (inputfd == -1) {
            perror("unable to open file for reading");
            exit(2);
        }
        dup2(inputfd, STDIN_FILENO);
    }
    if (outfile) {
        int outputfd = open(outfile, O_WRONLY | O_CREAT, 
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (outputfd == -1) {
            perror("unable to open file for writing");
            exit(3);
        }
        dup2(outputfd, STDOUT_FILENO);
    }
    if (appfile) {
        int appfd = open(appfile, O_WRONLY | O_APPEND);
        if (appfd == -1) {
            perror("unable to open file for appending");
            exit(4);
        }
        dup2(appfd, STDOUT_FILENO);
    }
}

void set_io_and_exec(struct command cmd, int pipe_fds_cur[], int pipe_fds_prev[]) 
{
    if(cmd.cmdflag & INPIP){
        if(setpgid(0, jobs.arr[jobs.last_id].lidpid) == -1) {
            perror("unable to place process in his job group");
            exit(1);
        }
    } else {
        if(setpgid(0, 0) == -1) {
            perror("unable to place process in his own group");
            exit(1);
        }
    }
    
    if (bkgrnd == 0 && (!(cmd.cmdflag & INPIP))) {
        tcsetpgrp(0, getpgrp()); // if it is first in fg, set group to fg 
    }

    set_files_io(); // redirection of input and output to files

    if (cmd.cmdflag & INPIP) {
        dup2(pipe_fds_prev[1], 0);
    }
    if (cmd.cmdflag & OUTPIP) {
        dup2(pipe_fds_cur[0], 1);
    }

    execvp(cmd.cmdargs[0], cmd.cmdargs);
}


int main()
{
    register int i;
    char line[MAXLINELEN];
    int ncmds;
    char prompt[] = "sh$ ";    
    tcgetattr(0, &shell_tattr);
    init_jobs();

    sigset(SIGINT, SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);
    sigset(SIGTTOU, SIG_IGN);
    sigset(SIGTTIN, SIG_IGN);
    sigset(SIGTSTP, SIG_IGN);
    sigset(SIGSTOP, SIG_IGN);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /*until eof  */
        int updcode = update_jobs();
        if ((ncmds = parseline(line)) <= 0) {
            if(updcode) {
                print_jobs(1);
            }
            continue;   /* read next line */
        }
        int pipe_fds_cur[2] = {0};
        int pipe_fds_prev[2] = {0};
        for (i = 0; i < ncmds; i++) {
            
            // parsing of jobs, fg and bg
            if(jobcontrol(cmds[i].cmdargs)) {
                continue;
            }

            // pipe creation, if needed
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
                sigset(SIGSTOP, SIG_DFL);
                
                set_io_and_exec(cmds[i], pipe_fds_cur, pipe_fds_prev);
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
                    ensure_joblist_size(); // cuz the joblist is growing array
                    jobs.arr[jobs.last_id].lidpid = chpid;
                    jobs.arr[jobs.last_id].stat = RUNNING;
                    jobs.arr[jobs.last_id].cnt_stopped = 0;
                    jobs.arr[jobs.last_id].cmdline[0] = 0;
                    if(bkgrnd) {
                        printf("[%d] %d\n", jobs.last_id, chpid);
                    }
                }
                int k = 0;
                while(1) {
                    // store command to job (append if process is not first in pipeline)
                    strcat(jobs.arr[jobs.last_id].cmdline, cmds[i].cmdargs[k]);
                    if(cmds[i].cmdargs[k+1]) {
                        strcat(jobs.arr[jobs.last_id].cmdline, " ");
                    } else {
                        break;
                    }
                    k++;
                }
                if(cmds[i].cmdflag & OUTPIP) {
                    strcat(jobs.arr[jobs.last_id].cmdline, " | ");
                }
                jobs.arr[jobs.last_id].cnt_process++;
                jobs.arr[jobs.last_id].cnt_running++;
                if(setpgid(chpid, jobs.arr[jobs.last_id].lidpid) == -1) {
                    perror("unable to place process in his job group from parent");
                    return 1;
                }
                if (!(cmds[i].cmdflag & OUTPIP)) {
                    // launch job in foreground (if we are on last process in pipeline)
                    if(bkgrnd == 0) {
                        job_to_fg(jobs.last_id, 2);
                    } else {
                        printf("id: %d\n", jobs.last_id);
                        stoplist_add(jobs.last_id);
                    }
                }
            }
        }
    }  /* close while */
}