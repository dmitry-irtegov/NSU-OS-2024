#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "shell.h"
#include <signal.h>
#include <errno.h>

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;
extern int errno;

int main(int argc, char *argv[])
{
    register int i;
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    char prompt[50];      /* shell prompt */
    int child_pid;

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /* until eof  */
        if ((ncmds = parseline(line)) <= 0)
            continue;   /* read next line */
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
            child_pid = fork();
            if (child_pid == -1) {
                fprintf(stderr, "fork() error\n");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) {
                int closed_in_fd = -1;
                int closed_out_fd = -1;
                if (i == 0 && infile) {
                    closed_in_fd = dup(0);
                    close(0);
                    if (open(infile, O_RDONLY) == -1) {
                        fprintf(stderr, "open() error\n");
                        exit(EXIT_FAILURE);
                    }
                }
                if (i == ncmds - 1 && outfile) {
                    closed_out_fd = dup(1);
                    close(1);
                    if (open(outfile, O_WRONLY | O_TRUNC) == -1) {
                        fprintf(stderr, "open() error\n");
                        exit(EXIT_FAILURE);
                    }
                } else if (i == ncmds - 1 && appfile) {
                    closed_out_fd = dup(1);
                    close(1);
                    if (open(appfile, O_WRONLY | O_APPEND) == -1) {
                        fprintf(stderr, "open() error\n");
                        exit(EXIT_FAILURE);
                    } 
                }

                if (!bkgrnd) {
                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);
                }

                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                
                if (closed_in_fd) {
                    close(0);
                    dup(closed_in_fd);
                }
                if (closed_out_fd) {
                    close(1);
                    dup(closed_out_fd);
                }

                exit(EXIT_SUCCESS);
            } else {
                if (bkgrnd) {
                    setpgid(child_pid, 0);
                    printf("pid of background process: %d\n", child_pid);
                } else {
                    int stat_lock;

                    if (waitpid(child_pid, &stat_lock, 0) == -1) {
                        fprintf(stderr, "waitpid() error\n");
                        exit(EXIT_FAILURE);
                    }

                    if (WEXITSTATUS(stat_lock)) {
                        fprintf(stderr, "process exit with code: %d\n",
                                WEXITSTATUS(stat_lock));
                    }
                }
            }
        }

    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */  
