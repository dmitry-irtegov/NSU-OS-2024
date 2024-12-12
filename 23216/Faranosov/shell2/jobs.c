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


extern struct command cmds[MAXCMDS];
extern struct convs conv[MAXCONV];
extern char bkgrnd, type;
extern int curcmd, maxJob;
extern job* firstjob, * lastjob;
extern int terminalfd;

extern char curDir[128];

extern char* specCommands[];

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