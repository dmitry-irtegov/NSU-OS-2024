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
job* firstjob, * lastjob, * jobForSpec, *nextJobForSpec;
int terminalfd;

char curDir[128];

char* specCommands[] = { "jobs", "fg", "bg", "exit", "cd"};

int setbgjob(job* curJob);
int setfgjob(job* curJob);
int setfd(streams* stream, char fdtocls);
command* copycmd(command* cmd);
convs* copyconv(convs* old);
process* initProc(command* cmd);
job* initJob(convs* conv);
job* linkConsAndJobs();
job* findJob(pid_t gpid);
job* findJobNumber(int number);
void clear(job* curj);
void clearAll();
void deleteJob(job* curj);
void handling_status(job* curJob, int status);
int printCurDir();
void printJob(job* j, int isDone);
void printJobs();
void findNextJobForSpec()

int checkJobs() {
	pid_t getted_id;
	int status;
	char shouldCont = 1;
	job* fj = NULL;
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
			return -1;
		default:
			fj = findJob(getted_id);
			handling_status(fj, status);
			break;
		}

	} while (shouldCont);

	return 0;
}

int shellawaiting(job* forgjob) {
	int status;
	job* fj = NULL;
	pid_t idToWait = forgjob->gpid, getted_id;
	for (;;) {
		getted_id = waitpid(-1, &status, WUNTRACED);
		switch (getted_id) {
		case -1:

			perror("wait (shallawaiting) error");
			return -1;

		default:
			if (getted_id == idToWait) {
				handling_status(forgjob, status);

				if (tcsetpgrp(terminalfd, getpid()) == -1) {
					perror("tcsetpgrp (shallawaiting) error");
					return -1;
				}

				return 0;
			}
			fj = findJob(getted_id);
			handling_status(fj, status);
			break;
		}
	}

	return 0;
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

int setsignal(int sig, void (*func)(int), char* procName) {
	if (sigset(sig, func) == SIG_ERR) {
		printf("%s\n", procName);
		perror("setsig error");
		return -1;
	}
	return 0;
}

int start_job(job* jobs) {
	if (jobs == NULL) return 0;

	jobs->state = RUNNING;
	pid_t sid;
	int status, pipes[2], infile, outfile, errfile, cntcmds, waitVal;
	infile = cntcmds = outfile = 0;
	if (jobs->conv->cntcommands == 1) {

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[1]) == 0) {
			if (jobs->proc->cmd->cmdargs[1] == NULL) {
				job* curJ = jobForSpec;
				if (curJ != NULL) {
					setfgjob(curJ);
					jobForSpec = nextJobForSpec;
					findNextJobForSpec();
				}
				else {
					printf("No cuurent job\n");
				}
			}
			else {
				int value = atoi(jobs->proc->cmd->cmdargs[1]);
				if (value <= 0) {
					printf("Job number must be positive\n");
					return 0;
				}
				job* fj = findJobNumber(value);
				if (fj == NULL) {
					printf("No such job\n");
					return 0;
				}
				for (process* p = fj->proc; p; p = p->nextproc) {
					for (int i = 0; p->cmd->cmdargs[i]; i++) {
						printf("%s ", p->cmd->cmdargs[i]);
					}
					if (p->nextproc != NULL) printf("| ");
				}
				printf("\n");
				setfgjob(fj);
			}
			deleteJob(jobs);
			return 0;
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[2]) == 0) {
			if (jobs->proc->cmd->cmdargs[1] == NULL) {
				job* curJ = jobForSpec;
				if (curJ != NULL) {
					setbgjob(curJ);
					jobForSpec = nextJobForSpec;
					findNextJobForSpec();
				}
				else {
					printf("No current job\n");
				}
			}
			else {
				int value = atoi(jobs->proc->cmd->cmdargs[1]);
				if (value <= 0) {
					printf("Job number must be positive\n");
					return 0;
				}
				job* fj = findJobNumber(value);
				if (fj == NULL) {
					printf("No such job\n");
					return 0;
				}
				for (process* p = fj->proc; p; p = p->nextproc) {
					for (int i = 0; p->cmd->cmdargs[i]; i++) {
						printf("%s ", p->cmd->cmdargs[i]);
					}
					if (p->nextproc != NULL) printf("| ");
				}
				printf("\n");
				setbgjob(fj); 
			}
			
			deleteJob(jobs);
			return 0;
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[3]) == 0) {
			clearAll();
			exit(0);
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[4]) == 0) {
			chdir(jobs->proc->cmd->cmdargs[1]);
			deleteJob(jobs);
			return 0;
		}
		
	}

	switch (sid = fork()) {
	case -1:
		perror("fork error");
		return -1;
	case 0:


		if (setpgid(getpid(), getpid()) == -1) {
			perror("setpgid (start_job) S");
			exit(1);
		}



		if (setsignal(SIGINT, SIG_DFL, "Son") == -1) exit(1);
		if (setsignal(SIGQUIT, SIG_DFL, "Son") == -1) exit(1);
		if (setsignal(SIGTTOU, SIG_IGN, "Son") == -1) exit(1);
		if (setsignal(SIGTSTP, SIG_DFL, "Son") == -1) exit(1);


		if (jobs->conv->flag != BKGRND) {
			if (tcsetpgrp(0, getpid()) == -1) {
				perror("S (start_job): tcsetpgrp error");
				exit(1);
			}
		}

		if (jobs->conv->err.flags & ISEXIST) {
			errfile = setfd(&jobs->conv->err, 2);
			if (errfile == -1) {
				exit(1);
			}
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
				if (infile == -1) {
					exit(1);
				}
			}

			if (!p->nextproc && jobs->conv->out.flags & ISEXIST) {
				outfile = setfd(&jobs->conv->out, 1);
				if (outfile == -1) {
					exit(1);
				}
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
					exit(1);
				}
				infile = 0;
			}
			if (outfile != 0) {
				if (close(outfile) == -1) {
					perror("S: close (outfile) error");
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
			if (shellawaiting(jobs) == -1) return -1;
		}
		else {
			printf("[%d] %d\n", jobs->number, jobs->gpid);
		}
		return 0;
	}
}

int setbgjob(job* curJob) {
	curJob->conv->flag = 1;

	if (curJob->state == STOPPED) {
		if (kill((-1) * curJob->gpid, SIGCONT) == -1) {
			perror("kill setbg");
			return -1;
		}
		curJob->state = RUNNING;
		curJob->conv->flag = BKGRND;
	}

	printf("[%d]", curJob->number);
	if (curJob == jobForSpec) printf("+ ");
	else if (curJob == nextJobForSpec) printf("- ");
	else printf(" ");

	for (process* p = curJob->proc; p; p = p->nextproc) {
		for (int i = 0; p->cmd->cmdargs[i]; i++) {
			printf("%s ", p->cmd->cmdargs[i]);
		}
		if (p->nextproc != NULL) printf("| ");
	}
	printf("\n");

	return 0;
}

int setfgjob(job* curJob) {
	if (tcsetpgrp(terminalfd, curJob->gpid) == -1) {
		perror("setfgjob tcsetpgrp");
		return -1;
	}

	curJob->conv->flag = 0;

	if (curJob->state == STOPPED) {
		if (kill((-1) * curJob->gpid, SIGCONT) == -1) {
			perror("kill setfg");
			return -1;
		}
		curJob->state = RUNNING;
		curJob->conv->flag = 0;
	}

	return shellawaiting(curJob);
}

void myexit() {
	clearAll();
	exit(1);
}

int main() {
	char line[1024];
	int ncmds;
	terminalfd = 0;
	job* jobToStart;
	firstjob = NULL;
	jobForSpec = NULL;
	nextJobForSpec = NULL;

	if (isatty(terminalfd) == 0) {
		perror("isatty error");
		exit(1);
	}



	if (setpgid(getpid(), getpid()) == -1) {
		perror("setpgid main");
		exit(1);
	}
	if (tcsetpgrp(terminalfd, getpid()) == -1) {
		perror("tcsetpgrp main");
		exit(1);
	}

	if (printCurDir() == -1) {
		myexit();
	}

	/* PLACE SIGNAL CODE HERE */

	if (setsignal(SIGINT, SIG_IGN, "mainIGN") == -1) myexit();
	if (setsignal(SIGQUIT, SIG_IGN, "mainIGN") == -1) myexit();
	if (setsignal(SIGTTIN, SIG_IGN, "mainIGN") == -1) myexit();
	if (setsignal(SIGTTOU, SIG_IGN, "mainIGN") == -1) myexit();
	if (setsignal(SIGTSTP, SIG_IGN, "mainIGN") == -1) myexit();

	for (;;) {


		if (promptline(line, 1024) == -1) {
			myexit();
		}
		if ((ncmds = parseline(line)) == 0) {
			checkJobs();
			if (printCurDir() == -1) {
				myexit();
			}
			continue;
		}
		else if (ncmds == -1) {
			myexit();
		}
		curcmd = 0;


		jobToStart = linkConsAndJobs();
		clearPars();

		if (checkJobs() == -1) myexit();
		

		
		curcmd = 0;
		for (job* j = jobToStart; j; j = j->nextjob) {
			if (start_job(j) == -1) myexit();
		}

		if (printCurDir() == -1) {
			myexit();
		}
	}

	if (setsignal(SIGINT, SIG_DFL, "mainIGN") == -1) myexit();
	if (setsignal(SIGQUIT, SIG_DFL, "mainIGN") == -1) myexit();
	if (setsignal(SIGTTIN, SIG_DFL, "mainIGN") == -1) myexit();
	if (setsignal(SIGTTOU, SIG_DFL, "mainIGN") == -1) myexit();
	if (setsignal(SIGTSTP, SIG_DFL, "mainIGN") == -1) myexit();;


	/* PLACE SIGNAL CODE HERE */
}
