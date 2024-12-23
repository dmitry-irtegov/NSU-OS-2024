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
char* ss = NULL;
char* sss = NULL;


void done(int stat) {
	clear_jobs();
	if (sss)
		free(sss);
	reset_terminal();
	kill_all();
	exit(stat);
}


void sigINT(int sig) {
	signal(SIGINT, sigINT);
	if (front) {
		kill(-front, SIGINT);
	}
	upd_job();
}

void sigQUIT(int sig) {
	signal(SIGQUIT, sigQUIT);
	if (front) {
		kill(-front, SIGQUIT);
	}
	upd_job();
}

void sigSTOP(int sig) {
	signal(SIGTSTP, sigSTOP);
	if (front) {
		kill(-front, SIGTSTP);
		pr_job(front);
		front = 0;
		bkgrnd = 0;
	}
	upd_job();
}

void sigCHLD(int sig) {
	signal(SIGCHLD, sigCHLD);
	upd_job();
}

int main(int argc, char* argv[]) {
	register int i;
	char line[1024];      /* Allow large command lines */
	int len;
	int ncmds;
	char prompt[1100];      /* Shell prompt */
	char cwd[1024];
	char home_cwd[1024];
	pid_t pgid = 0;

	//Две трубы для 3ных и более конвееров
	int pipefd[2] = { 0 };
	int pipefd1[2] = { 0 };

	init_jobs();

	/* PLACE SIGNAL CODE HERE */


	signal(SIGINT, sigINT);
	signal(SIGQUIT, sigQUIT);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, sigCHLD);
	signal(SIGTSTP, sigSTOP);

	//Директория
	getcwd(cwd, sizeof(cwd));
	getcwd(home_cwd, sizeof(home_cwd));
	snprintf(prompt, sizeof(prompt), "%s: %s> ", argv[0], cwd);

	set_default_termios();


	while ((len = promptline(prompt, line, sizeof(line))) >= 0) { /* Until EOF */\
		if (!len) continue;
		line[len] = '\n';
		line[len + 1] = '\0';
		if ((ncmds = parseline(line)) <= 0) continue;
		able_job_control();

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
			if (!sss)
				free(sss);
			ss = (char*)malloc(1024);
			sss = ss;
			memset(ss, 0, 1024);
			
			
			int mul = 1;
			//kill
			if (strcmp(cmds[i].cmdargs[0], "kill") == 0) {
				mul = -1;
			}

			pid_t pidd;
			for (int j = 0; cmds[i].cmdargs[j] != NULL; j++) {
				if (cmds[i].cmdargs[j][0] == '\0') {
					fprintf(stderr, "syntax error\n");
					continue;
				}
				if (cmds[i].cmdargs[j][0] == '%') {
					pidd = mul*((isdigit(*cmds[i].cmdargs[j])) ? get_g_int(atoi(cmds[i].cmdargs[j])) : get_g_ch(*cmds[i].cmdargs[j]));
					sprintf(ss, "%d", pidd);
					cmds[i].cmdargs[j] = ss;
					ss += strlen(ss) + 1;
				}
			}


			if (strcmp(cmds[i].cmdargs[0], "jobs") == 0) {  // Special case for "jobs"

				print_jobs();
				continue;  // No further parsing needed
			}

			if (strcmp(cmds[i].cmdargs[0], "fg") == 0) {  // Special case for "jobs"
				if (cmds[i].cmdargs[1] == NULL) {

					front = get_g_ch('+');
					if (to_fg(0) == -1) {
						fprintf(stderr, "Failed to move job to foreground\n");
					}
					front = 0;
				}
				else {
					front = atoi(cmds[i].cmdargs[1]);
					if (to_fg(get_job_id(strtol(cmds[i].cmdargs[1], NULL, 10))) == -1) {
						fprintf(stderr, "Failed to move job to foreground\n");
					}
					front = 0;
				}
				continue;  // No further parsing needed
			}

			if (strcmp(cmds[i].cmdargs[0], "bg") == 0) {  // Special case for "jobs"
				if (cmds[i].cmdargs[1]) {
					for (int ii = 1; cmds[i].cmdargs[ii]; ii++)
						if (to_bg(get_job_id(strtol(cmds[i].cmdargs[1], NULL, 10))) == -1) {
							fprintf(stderr, "Failed to move job to background\n");
						}
				}
				continue;  // No further parsing needed
			}

			//cd
			if (strcmp(cmds[i].cmdargs[0], "cd") == 0) {
				if (cmds[i].cmdargs[1] == NULL) {
					if (chdir(home_cwd)) {
						perror("cd");
					}
				}
				else if (chdir(cmds[i].cmdargs[1])) {
					perror("cd");
				}

				getcwd(cwd, sizeof(cwd));
				snprintf(prompt, sizeof(prompt), "%s: %s> ", argv[0], cwd);

				continue;
			}

			//quit
			if (strcmp(cmds[i].cmdargs[0], "quit") == 0 || strcmp(cmds[i].cmdargs[0], "q") == 0) {
				done(0);
			}

			if ((cmds[i].cmdflag & OUTPIP) && pipefd[0]) {//Открываем 2 пайп если первый занят
				if (pipe(pipefd1) == -1) {
					perror("pipe");
					done(-1);
				}

			}
			else if (cmds[i].cmdflag & OUTPIP) {//Иначе первый
				if (pipe(pipefd) == -1) {
					perror("pipe");
					done(-1);
				}
			}

			if ((pid = fork()) == 0) { // Дочерний процесс
				signal(SIGTERM, SIG_IGN);
				int fd;

				if (pipefd[0] && pipefd[1] && !pipefd1[0] && !pipefd1[1]) pgid = getpid();
				if (!pipefd[0] && !pipefd[1] && pipefd1[0] && pipefd1[1]) pgid = 0;
				setpgid(0, pgid);

				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);
				signal(SIGTTIN, SIG_DFL);

				// Перенаправление вывода (>>)
				if (appfile) {
					fd = open(appfile, O_CREAT | O_APPEND | O_WRONLY, 0777);
					if (fd < 0) {
						perror("Failed to open appfile");
						done(1);
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
						done(1);
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
						done(1);
					}
					dup2(fd, STDOUT_FILENO);
					close(fd);
					outfile = NULL;
				}

			#ifdef DEBUG
				fprintf(stderr, "<%d, %d + %d, %d - %d>\n", pipefd[0], pipefd[1], pipefd1[0], pipefd1[1], cmds[i].cmdflag);
			#endif


				if (cmds[i].cmdflag & INPIP) {//Входящая труба
					if (pipefd[0] && !pipefd[1]) {//Если первая труба занята
						dup2(pipefd[0], STDIN_FILENO);
					}
					else if (pipefd1[0] && !pipefd1[1]) {
						dup2(pipefd1[0], STDIN_FILENO);
					}
					else fprintf(stderr, "Pipe in error");
				}
				if (cmds[i].cmdflag & OUTPIP) {//Выходящая
					if (pipefd1[1] && pipefd1[0]) {
						dup2(pipefd1[1], STDOUT_FILENO);
						close(pipefd1[1]);
					}
					else if (pipefd[1] && pipefd[0]) {
						dup2(pipefd[1], STDOUT_FILENO);
						close(pipefd[1]);
					}
					else fprintf(stderr, "Pipe out error");
				}


				if (pipefd[0]) close(pipefd[0]);
				if (pipefd1[1]) close(pipefd1[1]);
				if (pipefd[1]) close(pipefd[1]);
				if (pipefd1[0]) close(pipefd1[0]);
				// Выполнение команды

				upd_job();
				for (int h = 0; cmds[i].cmdargs[h]; h++) fprintf(stderr, "%s ", cmds[i].cmdargs[h]);
				fprintf(stderr, "\n");
				execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
				perror("Execution error");
				done(1);
			}
			else if (pid > 0) {

				//Закрываем определенные использованные трубы
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

				if (!(cmds[i].cmdflag & OUTPIP)) {//Если не в конвейере
					int jb = add_job(cmds[i].cmdargs[0], pid, !bkgrnd);
					upd_job();
					reorder_priorities(jb);
					if (!bkgrnd) {
						front = pid;
						if (to_fg(jb) == -1) {
							fprintf(stderr, "Failed to move job to foreground\n");
						}
						front = 0;
					}
					else {
						upd_job();
						pr_job(pid);
					}
				}
				bkgrnd = 0;
			}
		}

		//Подгавливаем к следующей строке
		if (pipefd[0]) close(pipefd[0]);
		pipefd[0] = 0;
		if (pipefd1[1]) close(pipefd1[1]);
		pipefd1[1] = 0;
		if (pipefd1[0]) close(pipefd1[0]);
		pipefd1[0] = 0;
		if (pipefd[1]) close(pipefd[1]);
		pipefd[1] = 0;
		getcwd(cwd, sizeof(cwd));
		snprintf(prompt, sizeof(prompt), "%s: %s> ", argv[0], cwd);
	}  /* Close while */

	done(0);
}

