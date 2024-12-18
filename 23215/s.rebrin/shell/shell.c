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
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#define DEBUG

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
		pr_job(front);
		front = 0;
		bkgrnd = 0;
	}
}

void sigCHLD(int sig) {
	signal(SIGCHLD, sigCHLD);
	upd_job();
}

int main(int argc, char* argv[]) {
	register int i;
	char* line = NULL;      /* Allow large command lines */
	int ncmds;
	char prompt[1100];      /* Shell prompt */
	char cwd[1024];
	int pipefd[2] = { 0 };
	int pipefd1[2] = { 0 };

	init_jobs();

	/* PLACE SIGNAL CODE HERE */


	signal(SIGINT, sigINT);
	signal(SIGTSTP, sigSTOP);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, sigCHLD);

	getcwd(cwd, sizeof(cwd));
	snprintf(prompt, sizeof(prompt), "%s: %s> ", argv[0], cwd);

	set_default_termios();


	while ((line = readline(prompt)) != NULL) { /* Until EOF */
		size_t len = strlen(line);
		line[len] = '\n';
		line[len + 1] = '\0';
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
			if (strcmp(cmds[i].cmdargs[0], "jobs") == 0) {  // Special case for "jobs"
				print_jobs();
				continue;  // No further parsing needed
			}

			if (strcmp(cmds[i].cmdargs[0], "fg") == 0) {  // Special case for "jobs"
				if (cmds[i].cmdargs[1] == NULL) {

					front = pid;
					printf("ok");
					if (to_fg(0) == -1) {
						fprintf(stderr, "Failed to move job to foreground\n");
					}
					front = 0;
				}
				else {
					to_fg(get_job_id(strtol(cmds[i].cmdargs[1], NULL, 10)));
				}
				continue;  // No further parsing needed
			}

			if (strcmp(cmds[i].cmdargs[0], "cd") == 0) {
				if (cmds[i].cmdargs[1] == NULL) {
					fprintf(stderr, "cd: missing argument\n");
					continue;
				}

				if (chdir(cmds[i].cmdargs[1])) {
					perror("cd");
				}

				getcwd(cwd, sizeof(cwd));
				snprintf(prompt, sizeof(prompt), "%s: %s> ", argv[0], cwd);

				continue;
			}

			if (strcmp(cmds[i].cmdargs[0], "quit") == 0 || strcmp(cmds[i].cmdargs[0], "q") == 0) {
				clear_jobs();
				exit(0);
			}

			if (cmds[i].cmdflag & OUTPIP && pipefd[0]) {
				if (pipe(pipefd1) == -1) {
					perror("pipe");
					exit(-1);
				}

			}
			else if (cmds[i].cmdflag & OUTPIP) {
				if (pipe(pipefd) == -1) {
					perror("pipe");
					exit(-1);
				}
			}

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
				fprintf(stderr, "<%d, %d + %d, %d - %d>\n", pipefd[0], pipefd[1], pipefd1[0], pipefd1[1], cmds[i].cmdflag);
				if (cmds[i].cmdflag & INPIP) {
					if (pipefd[0]) {
						if (pipefd1[0]) close(pipefd1[0]);
						if (pipefd[1]) close(pipefd[1]);
						dup2(pipefd[0], STDIN_FILENO);
						close(pipefd[0]);
					}
					else if (pipefd1[0]) {
						if (pipefd[0]) close(pipefd[0]);
						if (pipefd1[1]) close(pipefd1[1]);
						if (pipefd[1]) close(pipefd[1]);
						dup2(pipefd1[0], STDIN_FILENO);
						close(pipefd1[0]);
					}
					else fprintf(stderr, "Pipe in error");
				}
				if (cmds[i].cmdflag & OUTPIP) {
					if (pipefd1[1]) {
						if (pipefd[1]) close(pipefd[1]);
						if (pipefd1[0]) close(pipefd1[0]);
						dup2(pipefd1[1], STDOUT_FILENO);
						close(pipefd1[1]);
					}
					else if (pipefd[1]) {
						if (pipefd1[1]) close(pipefd1[1]);
						if (pipefd1[0]) close(pipefd1[0]);
						if (pipefd[0]) close(pipefd[0]);
						dup2(pipefd[1], STDOUT_FILENO);
						close(pipefd[1]);
					}
					else fprintf(stderr, "Pipe out error");
				}

				// Выполнение команды

				upd_job();
				execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
				perror("Execution error");
				exit(1);
			}
			else if (pid > 0) {
				
				if (pipefd[1]) {
					close(pipefd[1]);
					pipefd[1] = 0;
				}
				else if (pipefd[0]) {
					close(pipefd[0]);
					pipefd[0] = 0;
				}
				
				if (pipefd1[1]) {
					close(pipefd1[1]);
					pipefd1[1] = 0;
				}
				else if (pipefd1[0]) {
					close(pipefd1[0]);
					pipefd1[0] = 0;
				}
				
				if (!(cmds[i].cmdflag & OUTPIP)) {
					int jb = add_job(cmds[i].cmdargs[0], pid, !bkgrnd);
					upd_job();
					reorder_priorities();
					if (!bkgrnd) {
						front = pid;
						if (to_fg(jb) == -1) {
							fprintf(stderr, "Failed to move job to foreground\n");
						}
						front = 0;
					}
					else
						pr_job(pid);
				}
					bkgrnd = 0;
			}
		}
		free(line);
		line = NULL;
		getcwd(cwd, sizeof(cwd));
		snprintf(prompt, sizeof(prompt), "%s: %s> ", argv[0], cwd);
	}  /* Close while */

	return 0;
}

/* PLACE SIGNAL CODE HERE */
