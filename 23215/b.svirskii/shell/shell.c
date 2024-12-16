#include <limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "shell.h"
#include <signal.h>
#include <errno.h>
#include "jobs.h"
#include <string.h>
#include <stdlib.h>

extern int errno;

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

Job* parse_job(char* line) {
    if (line == NULL) {
        fprintf(stderr, "can't identify job\n");
        return NULL;
    } else if (line[0] == '%') {
        if (line[1] == '\0') {
            return get_first_job();
        }
        int job_number = atoi(line + 1);
        if (job_number < 1) {
            fprintf(stderr, "invalid job number\n");
            return NULL;
        }
        return get_job(NUMBER, job_number);
    } else {
        int pid = atoi(line);
        if (pid < 1) {
            fprintf(stderr, "invalid job pid\n");
            return NULL;
        }
        return get_job(PID, pid);
    }
}
int main(int argc, char *argv[])
{
    if (!isatty(0)) {
        fprintf(stderr, "stdin doesn't associated with terminal\n");
        exit(EXIT_FAILURE);
    }
    if (!isatty(1)) {
        fprintf(stderr, "stdout doesn't associated with terminal\n");
        exit(EXIT_FAILURE);
    }
    register int i;
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    char prompt[50];      /* shell prompt */
    int child_pid;

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /* until eof  */
        if ((ncmds = parseline(line)) <= 0) {
            check_jobs_states_updates();
            continue;   /* read next line */
        }
#ifdef DEBUG
{
    for (int i = 0; i < ncmds; i++) {
        for (int j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++) {
            fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n", 
                    i, j, cmds[i].cmdargs[j]);
        }
        fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
    }
    fprintf(stderr, "infile: %s\n", infile);
    fprintf(stderr, "outfile: %s\n", outfile);
    fprintf(stderr, "appfile: %s\n", appfile);
}
#endif
        for (i = 0; i < ncmds; i++) {
            if (strcmp("jobs", cmds[i].cmdargs[0]) == 0) {
                print_jobs();
                continue;
            }
            if (strcmp("fg", cmds[i].cmdargs[0]) == 0) {
                Job* job = parse_job(cmds[i].cmdargs[1]);   
                if (job == NULL) continue;
                turn_to_foreground(job);
                int stat;
                if (waitpid(job->pid, &stat, WUNTRACED) == -1) {
                    perror("waitpid() failed");
                    exit(EXIT_FAILURE);
                }

                if (tcsetpgrp(0, getpgrp()) == -1) {
                    perror("tcsetpgrp() failed");
                    exit(EXIT_FAILURE);
                }

                if (WIFSTOPPED(stat)) {
                    stop_job(job);
                }

                continue;
            }
            if (strcmp("bg", cmds[i].cmdargs[0]) == 0) {
                Job* job = parse_job(cmds[i].cmdargs[1]);   
                turn_to_background(job);
                continue;
            }
            child_pid = fork();
            if (child_pid == -1) {
                perror("fork() failed");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) {
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);

                if (i == 0 && infile) {
                    close(0);
                    if (open(infile, O_RDONLY) == -1) {
                        perror("open() input file failed");
                        exit(EXIT_FAILURE);
                    }
                }
                if (i == ncmds - 1 && outfile) {
                    close(1);
                    if (open(outfile, O_WRONLY | O_TRUNC) == -1) {
                        perror("open() output file failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (i == ncmds - 1 && appfile) {
                    close(1);
                    if (open(appfile, O_WRONLY | O_APPEND) == -1) {
                        perror("open() append file failed");
                        exit(EXIT_FAILURE);
                    } 
                }

                if (execvp(cmds[i].cmdargs[0], cmds[i].cmdargs) == -1) {
                    perror(cmds[i].cmdargs[0]);
                    close(0);
                    close(1);
                    exit(EXIT_FAILURE);
                }
                close(0);
                close(1);
                exit(EXIT_SUCCESS);
            } else {
                setpgid(child_pid, 0);
                if (bkgrnd) {
                    create_job(child_pid, RUNNING, line);
                } else {
                    tcsetpgrp(0, getpgid(child_pid));
                    int stat;

                    if (waitpid(child_pid, &stat, WUNTRACED) == -1) {
                        perror("waitpid() failed");
                        exit(EXIT_FAILURE);
                    }

                    if (WIFSTOPPED(stat)) {
                        create_job(child_pid, STOPPED, line); 
                    }

                    tcsetpgrp(0, getpgrp());
#ifdef DEBUG
                    printf("tcgetpgrp(): %d\nshell pgid: %d\n",
                            tcgetpgrp(0), getpgrp());
#endif
                }
            }
        }
        check_jobs_states_updates();
    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */  
