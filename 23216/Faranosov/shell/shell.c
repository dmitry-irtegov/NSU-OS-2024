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

char* specCommands[] = {"fg", "bg", "exit"};

void setbgjob(job* curJob);

void setfgjob(job* curJob);

job* findJob(pid_t gpid);

void deleteJob(job* curj);

int setfd(streams *stream, char fdtocls) {
	switch (fdtocls) {
	case 0:
		return open(stream->file, O_RDONLY);
	case 1:
	case 2:
		if (stream->flags & ISCONT) {
			return open(stream->file, O_WRONLY | O_APPEND | O_CREAT);
		}
		else {
			return open(stream->file, O_WRONLY | O_CREAT);
		}
		break;
	default:
		printf("HOW???");
		exit(1);
	}
}

process* initProc(command *cmd) {
	process* newProc = malloc(sizeof(process));
	newProc->cmd = cmd;
	newProc->nextproc = NULL;
	return newProc;
}

job* initJob(convs *conv) {
	job* newjob = malloc(sizeof(job));
	newjob->conv = conv;
	newjob->nextjob = NULL;
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

	job* firstThisJob = NULL, *curJob = lastjob;
	char shouldDelete = 0;
	for (int i = 0; i < MAXCONV && conv[i].cntcommands > 0; i++) {
		shouldDelete = 0;
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

		for (process* p = curJob->proc; p; p = p->nextproc) {
			
			if (strcmp(p->cmd->cmdargs[0], specCommands[0]) == 0) {
				if (curJob->conv->cntcommands == 1) {
					setfgjob(findJob(atoi(p->cmd->cmdargs[1])));
				}
				shouldDelete = 1;
			}

			if (strcmp(p->cmd->cmdargs[0], specCommands[1]) == 0) {
				if (curJob->conv->cntcommands == 1) {
					setbgjob(findJob(atoi(p->cmd->cmdargs[1])));
				}
				shouldDelete = 1;
			}

			if (strcmp(p->cmd->cmdargs[0], specCommands[2]) == 0) {
				exit(0);
			}

		}

		if (shouldDelete) {
			deleteJob(curJob);
		}

		if (firstThisJob == NULL) {
			firstThisJob = curJob;
		}		
	}

	return firstjob;
}

job* findJob(pid_t gpid) {
	job* cur = firstjob;
	while (cur && cur->gpid != gpid) {
		cur = cur->nextjob;
	}

	return cur;
}

void clear(job* curj) {
	process* proc = curj->proc, *temp;
	while (proc) {
		temp = proc->nextproc;
		free(proc);
		proc = temp;
	}

	free(curj);
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
		deleteJob(curJob);
		return;
	}
	if (WIFSIGNALED(status)) {
		deleteJob(curJob);
		return;
	}
	if (WIFSTOPPED(status)) {
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
		getted_id = waitpid(-1, &status, WNOHANG);
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
	
	char whileWorking = 1;
	int status;
	pid_t idToWait = forgjob->gpid, getted_id;

	for (;;) {
		getted_id = wait(&status);
		switch (getted_id) {
		case -1:

			perror("wait frogjob error");
			exit(1);

		default:
			if (getted_id == idToWait) {
				handling_status(forgjob, status);

				tcsetpgrp(terminalfd, getpid());
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
		perror("fork error");
		exit(1);
	case 0:

		if (infile != 0) {
			dup2(infile, 0);
			close(infile);
		}
		if (outfile != 0) {
			dup2(outfile, 1);
			close(outfile);
		}

		execvp(proc->cmd->cmdargs[0], proc->cmd->cmdargs);
		perror("exec error");
		exit(1);
	default:
		return;
	}

}

void start_job(job* jobs) {
	if (jobs == NULL) return;
	pid_t sid;
	int status, pipes[2], infile, outfile, errfile, cntcmds, waitVal;
	infile = cntcmds = outfile = 0;

	
	switch (sid = fork()) {
	case -1:
		perror("fork error");
		exit(1);
	case 0:
		
		setpgid(getpid(), getpid());

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);


		if (jobs->conv->flag != BKGRND) {
			signal(SIGTTOU, SIG_IGN);
			tcsetpgrp(0, getpid());	
		}

		if (jobs->conv->err.flags & ISEXIST) {
			errfile = setfd(&jobs->conv->err, 2);
			dup2(errfile, 2);
			close(errfile);
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
				close(infile);
			}
			if (outfile != 0) {
				close(outfile);
			}
			infile = pipes[0];
		}


		while (cntcmds < jobs->conv->cntcommands) {
			waitVal = wait(&status);
			if (waitVal == -1) {
				
				if (errno == ECHILD) {
					break;
				}

				perror("wait error");
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
		if (jobs->conv->flag != BKGRND) {
			tcsetpgrp(terminalfd, getpid());
		}
		exit(0);
		break;
	default:
		jobs->gpid = sid;
		jobs->state = 0;
		if (jobs->conv->flag == 0) {
			shellawaiting(jobs);
		}
	}
}

void setbgjob(job* curJob) {
	if (curJob->state == STOPPED) {
		kill(curJob->gpid, SIGCONT);
		curJob->state = RUNNING;
		curJob->conv->flag = BKGRND;
	}
}

void setfgjob(job* curJob) {
	tcsetpgrp(terminalfd, curJob->gpid);

	if (curJob->state == STOPPED) {
		kill(curJob->gpid, SIGCONT);
		curJob->state = RUNNING;
		curJob->conv->flag = 0;
	}

	shellawaiting(curJob);
}

void printCurDir() {
	getcwd(curDir, 128);

	printf("%s>\n", curDir);
}


void printJobs() {

	for (job* j = firstjob; j; j = j->nextjob) {
		if (j->conv->in.flags & ISEXIST) printf("in = %s\n", j->conv->in.file);
		if (j->conv->out.flags & ISEXIST) printf("out = %s", j->conv->out.file);

		for (process* p = j->proc; p; p = p->nextproc) {
			if (p->nextproc == NULL) {
				printf("%s ", p->cmd->cmdargs[0]);
			}
			else printf("%s |", p->cmd->cmdargs[0]);
		}
		printf("\n");
	}
}

int main(int argc, char* argv) {
	register int i;
	char line[1024];
	int ncmds;
	char prompt[50];
	terminalfd = 0;
	job* jobToStart;

	setpgid(getpid(), getpid());
	tcsetpgrp(terminalfd, getpid());

	printCurDir();
	
	/* PLACE SIGNAL CODE HERE */
	
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	for (;;) {
		

		promptline(line, 1024);
		if ((ncmds = parseline(line)) <= 0) continue;
		curcmd = 0;
		jobToStart = linkConsAndJobs();
		/*
#ifdef  DEBUG
		{
			int i, j;
			for (i = 0; i < ncmds; i++) {
				for (j = 0; cmds[i].cmdargs[j] != (char*)NULL; j++)
					fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n", i, j, cmds[i].cmdargs[j]);
				fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
			}
	}
#endif //  DEBUG
*/
		curcmd = 0;
		for (job* j = jobToStart; j; j = j->nextjob) {
			start_job(j);
		}

		checkJobs();


		printCurDir();
	}
	
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	

	/* PLACE SIGNAL CODE HERE */
}