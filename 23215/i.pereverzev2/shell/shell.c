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


char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;


typedef struct jobs_s {
    struct job *arr;
    int arsz;
    int plus_id;
    int mins_id;
} jobsinfo;

jobsinfo jobs;


void to_fg(struct job* jb, int needcont) {
    //printf("setting %d to fg\n", jb->lidpid);
    tcsetpgrp(0, jb->lidpid);
    if(needcont) {
        if(sigsend(P_PGID, jb->lidpid, SIGCONT) == -1) {
            perror ("unable to send SIGCONT to continue job execution");
        }
    }
}
int main(int argc, char *argv[])
{
    register int i;
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    char prompt[50];      /* shell prompt */
    
    jobs.arsz = 1024;
    jobs.arr = calloc(jobs.arsz, sizeof(struct job));
    jobs.plus_id = -1;
    jobs.mins_id = -2;
    /* PLACE SIGNAL CODE HERE */

    sigset(SIGINT, SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);
    sigset(SIGTTOU, SIG_IGN);
    sigset(SIGTTIN, SIG_IGN);

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
                printf("child, pid: %d\n", getpid()); // for debug
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                if(setpgid(0, 0) == -1) {
                    perror("unable to place process in his own group from child");
                    return 1;
                }
                tcsetpgrp(0, getpgrp());
                if (infile) {
                    int inputfd = open(infile, O_RDONLY);
                    if (inputfd == -1) {
                        perror("unable to open file for reading");
                        return 2;
                    }
                    dup2(inputfd, 0);
                }
                if (outfile) {
                    int outputfd = open(outfile, O_WRONLY | O_CREAT);
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
                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                fprintf(stderr, "unable to execute %s\n", cmds[i].cmdargs[0]);
                return 5;
            } else if (chpid == -1) {
                // fork error
                perror("unable to fork, out of resources to create process");
            } else {
                // parent
		        jobs.plus_id++;
                jobs.mins_id++;
                jobs.arr[jobs.plus_id].stopped = 0;	
                jobs.arr[jobs.plus_id].lidpid = chpid;
		        printf("lidpid is %d\n", chpid);
                if(setpgid(chpid, chpid) == -1) {
                    perror("unable to place process in his own group from parent");
                    return 1;
                }
                if (bkgrnd == 0) {
                    // launch process in foreground
                    printf("launching in fg and waiting\n");
                    siginfo_t infop;
                    to_fg(jobs.arr + jobs.plus_id, 0);
                    pid_t code = waitid(P_PID, chpid, &infop, WSTOPPED | WEXITED);
                    if(code == -1 && errno != EINTR) {
                        perror("unable to wait termination of created process");
                    } else if (code == -1 && errno == EINTR) {
                        fprintf(stderr, "process was interrupted");
                    }
                    printf("try to return shell to foreground\n");
                    if(tcsetpgrp(0, getpgrp()) == -1) {
                        perror("unable to return shell to fg");
                    }
                    printf("waited, setted shell group to fg\n");
                    
                    printf("current fg pgrp is: %d, shell grp is %d\n", tcgetpgrp(0), getpgrp());
                    
                    // if(code == -1 && errno != EINTR) {
	                //     jobs.arr[jobs.plus_id].fgrnd = 0;
                    // } else {
	                //     jobs.arr[jobs.plus_id].fgrnd = 1;
                    //     printf("%d\n", chpid);
                    // }
                }
            }
        }

    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */



// void tobg() 
// {
//     printf("process %d now will be background\n", curjob.lidpid);
//     fflush(stdout);
//     if (jobs[]) {
//         if(setpgid(curjob.lidpid, 0) == -1) {
// 	        perror("unable to background");
// 	    }
//     }
// }
