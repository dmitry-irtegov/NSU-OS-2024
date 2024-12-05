#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include "shell.h"

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

int main(int argc, char *argv[]) {
    if (!argc)
        printf("No program to execute\n");
    register int i;
    char line[1024]; /*  allow large command lines  */
    int ncmds;
    char prompt[50]; /* shell prompt */
    /* PLACE SIGNAL CODE HERE */
    sprintf(prompt, "[%s] ", argv[0]);
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
                    if (infile) {
                        int inputfile = open(infile, O_CREAT | O_RDONLY);
                        if (inputfile == -1 || dup2(STDIN_FILENO, inputfile)) {
                            perror("STDIN redirection");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (outfile) {
                        int outputfile = open(outfile, O_CREAT | O_WRONLY);
                        if (outputfile == -1 || dup2(STDOUT_FILENO, outputfile)) {
                            perror("STDOUT redirection");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (appfile) {
                        int appendfile = open(appfile, O_CREAT | O_APPEND);
                        if (appendfile == -1 || dup2(STDOUT_FILENO, appendfile)) {
                            perror("STDOUT redirection");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (execvp(cmds[i].cmdargs[0], cmds[i].cmdargs)) {
                        perror("Cannot execute this command");
                        exit(EXIT_FAILURE);
                    }
                default: 
                    if (!bkgrnd) {
                        if (waitpid(processid, NULL, 0) == -1) {
                            perror("Troubles with waiting the child process");
                            exit(EXIT_FAILURE);
                        }
                    }
            }
        }
    } /* close while */
}
/* PLACE SIGNAL CODE HERE */
