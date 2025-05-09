#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

static int shellsetup();

command cmds[MAXCMDS];

static pid_t shellpid;
static pid_t childpid;
static int watistatus;

int main() {
    if (shellsetup()) {
        perror("Troubles with presetting");
        exit(EXIT_FAILURE);
    }
    register int i;
    char line[1024]; /*  allow large command lines  */
    int ncmds;
    char prompt[50]; /* shell prompt */
    /* PLACE SIGNAL CODE HERE */
    printf("Wellcome to the BSE - Best Shell Ever!!!\n");
    sprintf(prompt, "[%s] ", "bse");
    /*until eof  */
    int processid;
    while (promptline(prompt, line, sizeof(line)) > 0) { 
        
        if ((ncmds = parseline(line)) <= 0)
            continue; /* read next line */
#ifdef DEBUG
        {
            int i, j;
            for (i = 0; i < ncmds; i++) {
                for (j = 0; cmds[i].cmdargs[j] != (char *)NULL; j++)
                    fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
                            i, j, cmds[i].cmdargs[j]);
                fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
            }
        }
#endif
        for (i = 0; i < ncmds; i++) {
            /*  FORK AND EXECUTE  */
            switch (processid = fork()) {
                case -1: 
                    perror("Cannot fork");
                    break;
                case 0:
                    childpid = getpid();
                    if (setpgid(childpid, childpid) == -1) {
                        perror("Troubles with pid");
                        exit(EXIT_FAILURE);
                    }
                    if (cmds[i].bkgrnd == 0) {
                        if (!isatty(STDIN_FILENO)) {
                            perror("STDIN is not a correct tty!");
                            exit(EXIT_FAILURE);
                        }

                        sigset(SIGINT, SIG_DFL);
                        sigset(SIGQUIT, SIG_DFL);
                        sigset(SIGTSTP, SIG_DFL);
                        
                        if (tcsetpgrp(STDIN_FILENO, childpid) == -1) {
                            perror("Cannot get right to tc");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (cmds[i].infile) {
                        int inputfile = open(cmds[i].infile, O_RDONLY);
                        if (inputfile == -1 || dup2(inputfile, STDIN_FILENO) == -1) {
                            perror("STDIN redirection");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (cmds[i].outfile) {
                        int outputfile = open(cmds[i].outfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                        if (outputfile == -1 || dup2(outputfile, STDOUT_FILENO) == -1) {
                            perror("STDOUT redirection");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (cmds[i].appfile) {
                        int appendfile = open(cmds[i].appfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                        if (appendfile == -1 || dup2(appendfile, STDOUT_FILENO) == -1) {
                            perror("STDOUT redirection");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (execvp(cmds[i].cmdargs[0], cmds[i].cmdargs)) {
                        perror("Cannot execute this command");
                        break;
                    }
                default: 
                    if (cmds[i].bkgrnd) {
                        printf("Bg proc id: %d\n", processid);
                    } else {
                        if (waitpid(processid, &watistatus, WUNTRACED) == -1) {
                            perror("Troubles with waiting the child process");
                            exit(EXIT_FAILURE);
                        }
                        
                        if (tcsetpgrp(STDIN_FILENO, shellpid) == -1) {
                            perror("Cannot gets ct back!");
                            exit(EXIT_FAILURE);
                        }

                        if (WIFSTOPPED(watistatus))
                            printf("\nProcess %d was stopeed\n", (int)processid);
                    }
            }
        }
    } /* close while */
}
/* PLACE SIGNAL CODE HERE */

static int shellsetup() {
    sigignore(SIGQUIT);
    sigignore(SIGINT);
    sigignore(SIGTTIN);
    sigignore(SIGTTOU);
    sigignore(SIGTSTP);
    
    if (!isatty(STDIN_FILENO)) 
        return EXIT_FAILURE;

    shellpid = getpid();
    if (setpgid(shellpid, shellpid) == -1)
        return EXIT_FAILURE;

    if (tcsetpgrp(STDIN_FILENO, shellpid) == -1)
        return EXIT_FAILURE;

    return 0;
}
