#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include "shell.h"
#include <signal.h>

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;



int main(int argc, char *argv[])
{
    register int i;
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    char prompt[50];      /* shell prompt */

    /* PLACE SIGNAL CODE HERE */
    void quithandle();
    sigset(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /*until eof  */
        if ((ncmds = parseline(line)) <= 0)
            continue;   /* read next line */
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

        for (i = 0; i < ncmds; i++) {
            
            /*  FORK AND EXECUTE  */
            int chpid = fork();
            if (chpid == 0) {
                // child
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
                if (bkgrnd != 0) { 
                    if(setpgid(0, 0) == -1) {
                        perror("unable to execute in background");
                    }
                }
                if (infile) {
                    int inputfd = open(infile, O_RDONLY);
                    if (inputfd == -1) {
                        perror("unable to open file for reading");
                    }
                    dup2(inputfd, 0);
                }
                if (outfile) {
                    int outputfd = open(outfile, O_WRONLY | O_CREAT);
                    if (outputfd == -1) {
                        perror("unable to open file for writing");
                    }
                    dup2(outputfd, 1);
                }
                if (appfile) {
                    int appfd = open(appfile, O_WRONLY | O_APPEND);
                    if (appfd == -1) {
                        perror("unable to open file for appending");
                    }
                    dup2(appfd, 1);
                }
                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                fprintf(stderr, "unable to execute %s\n", cmds[i].cmdargs[0]);
            } else if (chpid == -1) {
                // fork error
                perror("unable to fork, out of resources to create process");
            } else {
                // parent
                if (bkgrnd == 0) {
                    pid_t code = waitpid(chpid, NULL, 0);
                    if(code == -1) {
                        perror("unable to wait termination of created process");
                    }
                } else {
                    printf("%d\n", chpid);
                }
            }
        }

    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */

void quithandle() {
    printf("sigquit was received\n");
    fflush(stdout);
}
