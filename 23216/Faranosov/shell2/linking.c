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