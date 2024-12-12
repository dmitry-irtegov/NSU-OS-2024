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

job* findJob(pid_t gpid);

void deleteJob(job* curj);

void printJob(job* j);

void printJobs();

int setfd(streams* stream, char fdtocls) {
	int fd = 0;
	switch (fdtocls) {
	case 0:
		fd = open(stream->file, O_RDONLY);
		if (fd == -1) {
			perror("open infile error");
			exit(1);
		}
		return fd;
	case 1:
	case 2:
		if (stream->flags & ISCONT) {
			fd = open(stream->file, O_WRONLY | O_APPEND | O_CREAT, 0660);
			if (fd == -1) {
				perror("open out/err fileAppend error");
				exit(1);
			}
			return fd;
		}
		else {
			fd = open(stream->file, O_WRONLY | O_CREAT, 0660);
			if (fd == -1) {
				perror("open out/err file error");
				exit(1);
			}
			return fd;
		}
		break;
	default:
		printf("HOW???");
		exit(1);
	}
}

command* copycmd(command* cmd) {
	command* newCom = NULL;
	newCom = malloc(sizeof(command));
	if (newCom == NULL) {
		printf("malloc (copycmd) error\n");
		exit(1);
	}

	newCom->cmdflag = cmd->cmdflag;


	for (int i = 0; i < MAXARGS; i++) {
		if (cmd->cmdargs[i] == NULL) {
			newCom->cmdargs[i] = (char*)NULL;
			break;
		}
		newCom->cmdargs[i] = NULL;
		char* str = malloc(sizeof(char) * (strlen(cmd->cmdargs[i]) + 1));
		strcpy(str, cmd->cmdargs[i]);
		newCom->cmdargs[i] = str;
		if (newCom->cmdargs[i] == NULL) {
			printf("malloc (newCom->cmdargs[%d]) error\n", i);
			exit(1);
		}
	}

	return newCom;
}

convs* copyconv(convs* old) {
	convs* newConv = NULL;
	newConv = malloc(sizeof(convs));
	if (newConv == NULL) {
		printf("malloc (copyconv) error");
		exit(1);
	}

	newConv->flag = old->flag;
	newConv->cntcommands = old->cntcommands;
	newConv->in.file = newConv->out.file = newConv->err.file = NULL;
	if (old->in.file) {
		newConv->in.file = malloc(sizeof(char) * (strlen(old->in.file) + 1));
		if (newConv->in.file == NULL) {
			printf("malloc (copyconv->in) error");
			exit(1);
		}
		strcpy(newConv->in.file, old->in.file);
	}
	else newConv->in.file = NULL;

	if (old->out.file) {
		newConv->out.file = malloc(sizeof(char) * (strlen(old->out.file) + 1));
		if (newConv->out.file == NULL) {
			printf("malloc (copyconv->out) error");
			exit(1);
		}
		strcpy(newConv->out.file, old->out.file);
	}
	else newConv->out.file = NULL;

	if (old->err.file) {
		newConv->err.file = malloc(sizeof(char) * (strlen(old->err.file) + 1));
		if (newConv->err.file == NULL) {
			printf("malloc (copyconv->err) error");
			exit(1);
		}
		strcpy(newConv->err.file, old->err.file);
	}
	else newConv->err.file = NULL;

	newConv->in.flags = old->in.flags;
	newConv->out.flags = old->out.flags;
	newConv->err.flags = old->err.flags;
	return newConv;
}

process* initProc(command* cmd) {
	process* newProc = NULL;
	newProc = malloc(sizeof(process));
	if (newProc == NULL) {
		printf("malloc (initProc) error");
		exit(1);
	}
	newProc->cmd = copycmd(cmd);
	newProc->nextproc = NULL;
	return newProc;
}

job* initJob(convs* conv) {
	job* newjob = NULL;
	newjob = malloc(sizeof(job));
	if (newjob == NULL) {
		printf("malloc initJob error");
		exit(1);
	}
	newjob->state = LINKED;
	newjob->gpid = 0;
	newjob->conv = copyconv(conv);
	newjob->nextjob = NULL;
	newjob->prevjob = NULL;
	process* curProc = NULL;
	for (int i = 0; i < conv->cntcommands; i++) {
		if (i == 0) {
			curProc = initProc(&cmds[curcmd]);
			newjob->proc = curProc;
		}
		else {
			curProc->nextproc = initProc(&cmds[curcmd + i]);
			curProc = curProc->nextproc;
		}
	}
	curcmd += conv->cntcommands;
	return newjob;
}

job* linkConsAndJobs() {

	job* firstThisJob = NULL, * curJob = lastjob;
	for (int i = 0; i < MAXCONV && conv[i].cntcommands > 0; i++) {
		if (curJob == NULL) {
			curJob = initJob(&conv[i]);
			curJob->prevjob = NULL;
			firstjob = curJob;
			lastjob = curJob;
		}
		else {
			curJob->nextjob = initJob(&conv[i]);
			curJob->nextjob->prevjob = curJob;
			curJob = curJob->nextjob;
			lastjob = curJob;

		}

		if (curJob->conv->cntcommands != 1) {
			for (process* p = curJob->proc; p; p = p->nextproc) {

				if (strcmp(p->cmd->cmdargs[0], specCommands[3]) == 0) {
					if (curJob->conv->cntcommands != 1) {
						printf("Exit in conv: Undfined behavior!");
						exit(1);
					}
				}

			}

		}


		if (firstThisJob == NULL) {
			firstThisJob = curJob;
		}
	}

	
	return firstThisJob;
}

job* findJob(pid_t gpid) {
	job* cur = firstjob;
	while (cur && cur->gpid != gpid) {
		cur = cur->nextjob;
	}

	return cur;
}

void clear(job* curj) {
	process* proc = curj->proc, * temp;
	while (proc) {
		temp = proc->nextproc;
		free(proc->cmd);
		free(proc);
		proc = temp;
	}
	free(curj->conv);
	free(curj);
}

void clearAll() {
	for (job* j = firstjob; j != NULL; j = j->nextjob) {
		clear(j);
	}
}

void deleteJob(job* curj) {
	
	if (curj->nextjob) {
		curj->nextjob->prevjob = curj->prevjob;
	}
	else {
		lastjob = curj->prevjob;
	}
	if (curj->prevjob) {
		curj->prevjob->nextjob = curj->nextjob;
	}
	else {
		firstjob = curj->nextjob;
	}


	clear(curj);
}

void handling_status(job* curJob, int status) {
	if (WIFEXITED(status)) {
		if (curJob->conv->flag == BKGRND) printf("gpid = %d exited\n", curJob->gpid);
		deleteJob(curJob);
		return;
	}
	if (WIFSIGNALED(status)) {
		printf("gpid = %d killed by signal\n", curJob->gpid);
		deleteJob(curJob);
		return;
	}
	if (WIFSTOPPED(status)) {
		printf("gpid = %d stopped by signal\n", curJob->gpid);
		curJob->state = STOPPED;
		return;
	}

}

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
		printf("cnt commands == 1\n %s\n", jobs->proc->cmd[0].cmdargs[0]);

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[1]) == 0) {
			if (jobs->proc->cmd->cmdargs[1] == NULL) {
				job* curJ = lastjob;
				while (curJ != NULL || curJ->conv->flag == 0) {
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
			return;
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[2]) == 0) {
			setbgjob(findJob(atoi(jobs->proc->cmd->cmdargs[1])));
			return;
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[3]) == 0) {
			clearAll();
			exit(0);
		}

		if (strcmp(jobs->proc->cmd->cmdargs[0], specCommands[4]) == 0) {
			chdir(jobs->proc->cmd->cmdargs[1]);
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
	printf("gpid = %d\n", curJob->gpid);
	tcsetpgrp(terminalfd, curJob->gpid);


	if (curJob->state == STOPPED) {
		printf("Send SIGCONT\n");
		kill((-1) * curJob->gpid, SIGCONT);
		curJob->state = RUNNING;
		curJob->conv->flag = 0;
		printf("Sent SIGCONT\n");
	}

	printf("Start waiting\n");
	shellawaiting(curJob);
}

void printCurDir() {
	for (int i = 0; i < 128; i++) curDir[i] = '\0';
	getcwd(curDir, 128);

	if (write(1, curDir, 128) == -1) {
		perror("write error");
		exit(1);
	}

	if (write(1, "> ", 2) == -1) {
		perror("write error");
		exit(1);
	}

}

void printJob(job* j) {
	printf("gpid = %d  ", j->gpid);

	if (j->state == RUNNING) {
		printf("Running ");
	}
	else if (j->state == STOPPED) {
		printf("Stopped ");
	}
	else printf("Linked ");

	for (process* p = j->proc; p; p = p->nextproc) {
		for (int i = 0; p->cmd->cmdargs[i]; i++) {
			printf("%s ", p->cmd->cmdargs[i]);
		}
		if (p->nextproc != NULL) printf("| ");
	}
	printf("\n");
}

void printJobs() {
	printf("\n");
	int cnt = 0;
	for (job* j = firstjob; j; j = j->nextjob) {
		if (j->state != 3) {
			cnt++;
			printJob(j);
		}
	}
	printf("total: %d\n\n", cnt);
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
		if ((ncmds = parseline(line)) <= 0) continue;
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
