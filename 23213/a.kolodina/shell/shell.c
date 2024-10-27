#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include "shell.h"

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

main(int argc, char *argv[])
{
    register int i;
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    char prompt[50];      /* shell prompt */

    /* PLACE SIGNAL CODE HERE */

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /*
until eof  */
        if ((ncmds = parseline(line)) <= 0)
            continue;   /* read next line */
#ifdef DEBUG
{
    int i, j;
    for (i = 0; i < ncmds; i++) {
        for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
            fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
            i, j, cmds[i].cmdargs[j]);
        fprintf(stderr, "cmds[%d].cmdflag = %o\n", i,
cmds[i].cmdflag);
    }
}
#endif
        for (i = 0; i < ncmds; i++) {
            /*  FORK AND EXECUTE  */
            pid_t pid;
            pid = fork();
            if (pid == -1) {
                perror("fork error");
                return 1;
            } else if (pid == 0) {
                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                perror("execvp error");
                return 1;
            } else {
                int wstatus;
                if (waitpid(pid, &wstatus, 0) == -1) {
                    perror("waitpid error");
                    return 1;
                }
                if (WIFEXITED(wstatus)){
                    printf("exited, status = %d\n", WEXITSTATUS(wstatus));
                }
            }
        }

    }  /* close while */
}

 /* PLACE SIGNAL CODE HERE */
