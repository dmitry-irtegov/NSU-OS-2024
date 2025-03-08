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
extern job* firstjob, * lastjob, *jobForSpec, *nextJobForSpec;
extern int terminalfd;

extern char curDir[128];

extern char* specCommands[];


int printCurDir() {
	for (int i = 0; i < 128; i++) curDir[i] = '\0';
	getcwd(curDir, 128);

	if (write(1, curDir, 128) == -1) {
		perror("write error");
		return -1;
	}

	if (write(1, "> ", 2) == -1) {
		perror("write error");
		return -1;
	}

	return 0;
}

void printJob(job* j, int isDone) {
	printf("[%d]", j->number);
	if (j == jobForSpec) printf("+ ");
	else if (j == nextJobForSpec) printf("- ");
	else printf(" ");

	if (!isDone) {
		if (j->state == RUNNING) {
			printf("Running\t\t");
		}
		else if (j->state == STOPPED) {
			printf("Stopped\t\t");
		}
		else printf("Linked\t\t");
	}
	else {
		printf("Done\t\t");
	}
	for (process* p = j->proc; p; p = p->nextproc) {
		for (int i = 0; p->cmd->cmdargs[i]; i++) {
			printf("%s ", p->cmd->cmdargs[i]);
		}
		if (p->nextproc != NULL) printf("| ");
	}
	printf("\n");
}

void printJobs() {
	int cnt = 0;
	for (job* j = firstjob; j; j = j->nextjob) {
		if (j->state != 3 && j->gpid != 0) {
			cnt++;
			printJob(j, 0);
		}
	}
}