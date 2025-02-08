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
extern int curNumber;

extern struct command cmds[MAXCMDS];
extern struct convs conv[MAXCONV];
extern char bkgrnd, type;
extern int curcmd, maxJob;
extern job* firstjob, * lastjob, *jobForSpec, *nextJobForSpec;
extern int terminalfd;

extern char curDir[128];

extern char* specCommands[];

void printJob(job* j, int osDone);

job* findJob(pid_t gpid) {
	job* cur = firstjob;
	while (cur && cur->gpid != gpid) {
		cur = cur->nextjob;
	}

	return cur;
}

job* findJobNumber(int number) {
	job* cur = firstjob;
	while (cur && cur->number != number) {
		cur = cur->nextjob;
	}

	return cur;
}

void clear(job* curj) {
	if (curj == NULL) return;
	process* proc = curj->proc, * temp;
	while (proc) {
		temp = proc->nextproc;
		free(proc->cmd);
		free(proc);
		proc = temp;
	}
	if (curj->conv != NULL) free(curj->conv);
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

void findNextJobForSpec() {
	job* j = lastjob;
	while (j && (j == jobForSpec || j->conv->flag == 0 || j->gpid == 0 || j->number == 0)) {
		j = j->prevjob;
	}
	nextJobForSpec = j;
}

void handling_status(job* curJob, int status) {
	if (WIFEXITED(status)) {
		if (curJob->conv->flag == BKGRND) printJob(curJob, 1);
		if (curJob == jobForSpec) {
			jobForSpec = nextJobForSpec;
			findNextJobForSpec();
		}
		if (curJob == nextJobForSpec) {
			findNextJobForSpec();
		}
		deleteJob(curJob);
		return;
	}
	if (WIFSIGNALED(status)) {
		printf("\n");
		if (curJob == jobForSpec) {
			jobForSpec = nextJobForSpec;
			findNextJobForSpec();
		}
		if (curJob == nextJobForSpec) {
			findNextJobForSpec();
		}
		deleteJob(curJob);
		return;
	}
	if (WIFSTOPPED(status)) {
		printf("\n");
		curJob->state = STOPPED;
		curJob->conv->flag = 1;
		if (curJob->number == -1) {
			curJob->number = curNumber++;
		}
		if (jobForSpec == NULL) {
			jobForSpec = curJob;
		} 
		else if (nextJobForSpec == NULL) {
			nextJobForSpec = curJob;
		}
	
		printJob(curJob, 0);
		return;
	}

}