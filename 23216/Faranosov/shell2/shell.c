#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "shell.h"
#include "structures.h"
extern int errno;

struct command cmds[MAXCMDS];
struct convs conv[MAXCONV];
char bkgrnd, type;
int curcmd, maxJob;
job* firstjob, * lastjob;
int terminalfd;

char curDir[128];

char* specCommands[] = { "jobs", "fg", "bg", "exit", "cd"};

void setbgjob(job* curJob);
void setfgjob(job* curJob);
int setfd(streams* stream, char fdtocls);
command* copycmd(command* cmd);
convs* copyconv(convs* old);
process* initProc(command* cmd);
job* initJob(convs* conv);
job* linkConsAndJobs();
job* findJob(pid_t gpid);
void clear(job* curj);
void clearAll();
void deleteJob(job* curj);
void handling_status(job* curJob, int status);
void printCurDir();
void printJob(job* j);
void printJobs();

void checkJobs() {
	pid_t getted_id;
	int status;
	char shouldCont = 1;
	do
	{
		getted_id = waitpid(-1, &status, WNOHANG | WUNTRACED);
		switch (getted_id) {
		case 0:
			shouldCont = 0;
			break;
		case -1:
			if (errno == ECHILD) {
				shouldCont = 0;
				break;
			}
			perror("wait (checkJobs) error");
			exit(1);
		default:
			handling_status(findJob(getted_id), status);
			break;
		}

	} while (shouldCont);
}

void shellawaiting(job* forgjob) {
	int status;
	pid_t idToWait = forgjob->gpid, getted_id;
	for (;;) {
		getted_id = waitpid(-1, &status, WUNTRACED);
		switch (getted_id) {
		case -1:

			perror("wait (shallawaiting) error");
			exit(1);

		default:
			if (getted_id == idToWait) {
				handling_status(forgjob, status);

				if (tcsetpgrp(terminalfd, getpid()) == -1) {
					perror("tcsetpgrp (shallawaiting) error");
					exit(1);
				}

				return;
			}

			handling_status(findJob(getted_id), status);
			break;
		}
	}
}

void start_proc(process* proc, int infile, int outfile) {

	switch (fork()) {
	case -1:
		perror("fork (start_proc) error");
		exit(1);
	case 0:
		if (infile != 0) {
			if (dup2(infile, 0) == -1) {
				perror("SS: dup2 (in) error");
				exit(1);
			}
			if (close(infile) == -1) {
				perror("SS: close (in) error");
				exit(1);
			}
		}
		if (outfile != 0) {
			if (dup2(outfile, 1) == -1) {
				perror("SS: dup2 (out) error");
				exit(1);
			}
			if (close(outfile) == -1) {
				perror("SS: close(out) error");
				exit(1);
			}
		}

		if (strcmp(proc->cmd->cmdargs[0], specCommands[0]) == 0) {
			printJobs();
			exit(0);
		}


		execvp(proc->cmd->cmdargs[0], proc->cmd->cmdargs);
		perror("SS: exec error");
		exit(1);
	default:
		return;
	}

}

void setsignal(int sig, void (*func)(int), char* procName) {
	if (signal(sig, func) == SIG_ERR) {
		printf("%s\n", procName);
		perror("setsig error");
		exit(1);
	}
}

void start_job(job* jobs) {
	jobs->state = RUNNING;

	if (jobs == NULL) return;
	pid_t sid;
	int status, pipes[2], infile, outfile, errfile, cntcmds, waitVal;
	infile = cntcmds = outfile = 0;
	if (jobs->conv->cntcommands == 1) {

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[1]) == 0) {
			if (jobs->proc->cmd->cmdargs[1] == NULL) {
				job* curJ = lastjob;
				while (curJ != NULL && curJ->state != 1) {
					curJ = curJ->prevjob;
				}
				if (curJ != NULL) {
					setfgjob(curJ);
				}
			}
			else {
				int value = atoi(jobs->proc->cmd->cmdargs[1]);
				if (value == 0) {
					printf("atoi error");
					exit(1);
				}
				setfgjob(findJob(value));
			}
			deleteJob(jobs);
			return;
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[2]) == 0) {
			setbgjob(findJob(atoi(jobs->proc->cmd->cmdargs[1])));
			deleteJob(jobs);
			return;
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[3]) == 0) {
			clearAll();
			exit(0);
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[4]) == 0) {
			chdir(jobs->proc->cmd->cmdargs[1]);
			deleteJob(jobs);
			return;
		}
		
	}

	switch (sid = fork()) {
	case -1:
		perror("fork error");
		exit(1);
	case 0:


		if (setpgid(getpid(), getpid()) == -1) {
			perror("setpgid (start_job) S");
			exit(1);
		}



		setsignal(SIGINT, SIG_DFL, "Son");
		setsignal(SIGQUIT, SIG_DFL, "Son");
		setsignal(SIGTTOU, SIG_IGN, "Son");
		setsignal(SIGTSTP, SIG_DFL, "Son");


		if (jobs->conv->flag != BKGRND) {
			if (tcsetpgrp(0, getpid()) == -1) {
				perror("S (start_job): tcsetpgrp error");
				exit(1);
			}
		}

		if (jobs->conv->err.flags & ISEXIST) {
			errfile = setfd(&jobs->conv->err, 2);
			if (dup2(errfile, 2) == -1) {
				perror("S (start_job): dup2 (err) error");
				exit(1);
			}
			if (close(errfile) == -1) {
				perror("S (start_job): close (err) error");
				exit(1);
			}
		}



		for (process* p = jobs->proc; p; p = p->nextproc) {
			if (p == jobs->proc && jobs->conv->in.flags & ISEXIST) {
				infile = setfd(&jobs->conv->in, 0);
			}

			if (!p->nextproc && jobs->conv->out.flags & ISEXIST) {
				outfile = setfd(&jobs->conv->out, 1);
			}

			if (p->nextproc) {
				if (pipe(pipes) < 0) {
					perror("pipe error");
					exit(1);
				}
				outfile = pipes[1];
			}

			start_proc(p, infile, outfile);

			if (infile != 0) {
				if (close(infile) == -1) {
					perror("S: close (infile) error");
					printf("\n");
					exit(1);
				}
				infile = 0;
			}
			if (outfile != 0) {
				if (close(outfile) == -1) {
					perror("S: close (outfile) error");
					printf("\n");
					exit(1);
				}
				outfile = 0;
			}
			infile = pipes[0];
		}


		while (cntcmds < jobs->conv->cntcommands) {
			waitVal = waitpid(-1, &status, WUNTRACED);
			if (waitVal == -1) {

				if (errno == ECHILD) {
					break;
				}

				exit(1);
			}
			else {
				if (WIFEXITED(status)) {
					if (WEXITSTATUS(status) != 0) {
						exit(1);
					}
				}
				cntcmds++;
			}
		}
		exit(0);
		break;
	default:
		jobs->gpid = sid;
		jobs->state = 0;
		if (jobs->conv->flag == 0) {
			shellawaiting(jobs);
		}
		return;
	}
}

void setbgjob(job* curJob) {

	if (curJob->state == STOPPED) {
		kill((-1) * curJob->gpid, SIGCONT);
		curJob->state = RUNNING;
		curJob->conv->flag = BKGRND;
	}
}

void setfgjob(job* curJob) {
	tcsetpgrp(terminalfd, curJob->gpid);


	if (curJob->state == STOPPED) {
		kill((-1) * curJob->gpid, SIGCONT);
		curJob->state = RUNNING;
		curJob->conv->flag = 0;
	}

	shellawaiting(curJob);
}


int main() {
	char line[1024];
	int ncmds;
	terminalfd = 0;
	job* jobToStart;
	
	if (isatty(terminalfd) == 0) {
		perror("isatty error");
		exit(1);
	}



	setpgid(getpid(), getpid());
	tcsetpgrp(terminalfd, getpid());

	printCurDir();

	/* PLACE SIGNAL CODE HERE */

	setsignal(SIGINT, SIG_IGN, "mainIGN");
	setsignal(SIGQUIT, SIG_IGN, "mainIGN");
	setsignal(SIGTTIN, SIG_IGN, "mainIGN");
	setsignal(SIGTTOU, SIG_IGN, "mainIGN");
	setsignal(SIGTSTP, SIG_IGN, "mainIGN");

	for (;;) {


		promptline(line, 1024);
		if ((ncmds = parseline(line)) <= 0) {
			continue;
		}
		curcmd = 0;


		jobToStart = linkConsAndJobs();
		clearPars();

		checkJobs();
		

		
		curcmd = 0;
		for (job* j = jobToStart; j; j = j->nextjob) {
			start_job(j);
		}

		printCurDir();
	}

	setsignal(SIGINT, SIG_DFL, "mainDFL");
	setsignal(SIGQUIT, SIG_DFL, "mainDFL");
	setsignal(SIGTTIN, SIG_DFL, "mainDFL");
	setsignal(SIGTTOU, SIG_DFL, "mainDFL");
	setsignal(SIGTSTP, SIG_DFL, "mainDFL");


	/* PLACE SIGNAL CODE HERE */
}
