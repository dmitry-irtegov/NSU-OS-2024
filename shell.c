#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "shell.h"

//#define DEBUG

char* infile, * outfile, * appfile;
struct command cmds[MAXCMDS];
char bkgrnd;
int front = 0;


void sigINT(int sig) {
    signal(SIGINT, sigINT);
    if (front) {
        kill(front, SIGINT);
    }
}

void sigSTOP(int sig) {
    signal(SIGTSTP, sigSTOP);
    if (front) {
        printf("%d\n", front);
        front = 0;
        bkgrnd = 0;
    }
}

int main(int argc, char* argv[]) {
	register int i;
	char line[1024];      /* Allow large command lines */
	int ncmds;
	char prompt[50];      /* Shell prompt */

    init_jobs();

	/* PLACE SIGNAL CODE HERE */

	sprintf(prompt, "[%s] ", argv[0]);

    signal(SIGINT, sigINT);
    signal(SIGTSTP, sigSTOP);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

	while (promptline(prompt, line, sizeof(line)) > 0) { /* Until EOF */
		if ((ncmds = parseline(line)) <= 0) {
			continue;   /* Read next line */
		}

#ifdef DEBUG
		{
			int i, j;
			for (i = 0; i < ncmds; i++) {
				for (j = 0; cmds[i].cmdargs[j] != (char*)NULL; j++) {
					fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n", i, j, cmds[i].cmdargs[j]);
				}
				fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
			}
		}
#endif


		for (i = 0; i < ncmds; i++) {
			pid_t pid;

            if ((pid = fork()) == 0) { // Дочерний процесс

                int fd;

                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);

                // Перенаправление вывода (>>)
                if (appfile) {
                    fd = open(appfile, O_CREAT | O_APPEND | O_WRONLY, 0777);
                    if (fd < 0) {
                        perror("Failed to open appfile");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    appfile = NULL;
                }

                // Перенаправление ввода (<)
                if (infile) {
                    fd = open(infile, O_RDONLY);
                    if (fd < 0) {
                        perror("Failed to open infile");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    infile = NULL;
                }

                // Перенаправление вывода (>)
                if (outfile) {
                    fd = open(outfile, O_CREAT | O_TRUNC | O_WRONLY, 0777);
                    if (fd < 0) {
                        perror("Failed to open outfile");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    outfile = NULL;
                }

                // Выполнение команды

                upd_job();
                //fprintf(stderr, "ALOOOOO BLYAT");
                //fprintf(stderr, "{%d}", getpid());
                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                perror("Execution error");
                exit(1); 
            }
			else if (pid > 0) {
                int jb = add_job(cmds[i].cmdargs[0], pid, !bkgrnd);
                
                if (!bkgrnd) {
                    front = pid;
                    if (to_fg(jb) == -1) {
                        fprintf(stderr, "Failed to move job to foreground\n");
                    }
                    front = 0;  
                }
				else
					printf("%d\n", pid);
				bkgrnd = 0;
			}
		}
	}  /* Close while */

	return 0;
}

/* PLACE SIGNAL CODE HERE */
