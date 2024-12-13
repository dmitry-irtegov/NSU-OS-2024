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

extern void clear(job* curj);

int setfd(streams* stream, char fdtocls) {
	int fd = 0;
	switch (fdtocls) {
	case 0:
		fd = open(stream->file, O_RDONLY);
		if (fd == -1) {
			perror("open infile error");
			return -1;
		}
		return fd;
	case 1:
	case 2:
		if (stream->flags & ISCONT) {
			fd = open(stream->file, O_WRONLY | O_APPEND | O_CREAT, 0660);
			if (fd == -1) {
				perror("open out/err fileAppend error");
				return -1;
			}
			return fd;
		}
		else {
			fd = open(stream->file, O_WRONLY | O_CREAT | O_TRUNC, 0660);
			printf("%s\n", stream->file);
			if (fd == -1) {
				perror("open out/err file error");
				return -1;
			}
			return fd;
		}
		break;
	default:
		printf("HOW???");
		return -1;
	}
}

command* copycmd(command* cmd) {
	command* newCom = NULL;
	newCom = malloc(sizeof(command));
	if (newCom == NULL) {
		printf("malloc (copycmd) error\n");
		return NULL;
	}

	newCom->cmdflag = cmd->cmdflag;


	for (int i = 0; i < MAXARGS; i++) {
		if (cmd->cmdargs[i] == NULL) {
			newCom->cmdargs[i] = (char*)NULL;
			break;
		}
		newCom->cmdargs[i] = NULL;
		char* str = malloc(sizeof(char) * (strlen(cmd->cmdargs[i]) + 1));
		if (cmd->cmdargs[i]) strcpy(str, cmd->cmdargs[i]);
		else str = NULL;
		newCom->cmdargs[i] = str;
		if (newCom->cmdargs[i] == NULL) {
			printf("malloc (newCom->cmdargs[%d]) error\n", i);
			return NULL;
		}
	}

	return newCom;
}

convs* copyconv(convs* old) {
	convs* newConv = NULL;
	newConv = malloc(sizeof(convs));
	if (newConv == NULL) {
		printf("malloc (copyconv) error");
		return NULL;
	}

	newConv->flag = old->flag;
	newConv->cntcommands = old->cntcommands;
	newConv->in.file = newConv->out.file = newConv->err.file = NULL;
	if (old->in.file) {
		newConv->in.file = malloc(sizeof(char) * (strlen(old->in.file) + 1));
		if (newConv->in.file == NULL) {
			printf("malloc (copyconv->in) error");
			free(newConv);
			return NULL;
		}
		strcpy(newConv->in.file, old->in.file);
	}
	else newConv->in.file = NULL;

	if (old->out.file) {
		newConv->out.file = malloc(sizeof(char) * (strlen(old->out.file) + 1));
		if (newConv->out.file == NULL) {
			printf("malloc (copyconv->out) error");
			free(newConv);
			return NULL;
		}
		strcpy(newConv->out.file, old->out.file);
	}
	else newConv->out.file = NULL;

	if (old->err.file) {
		newConv->err.file = malloc(sizeof(char) * (strlen(old->err.file) + 1));
		if (newConv->err.file == NULL) {
			printf("malloc (copyconv->err) error");
			free(newConv);
			return NULL;
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
		return NULL;
	}
	newProc->cmd = copycmd(cmd);
	if (newProc->cmd == NULL) {
		free(newProc);
		return NULL;
	}
	newProc->nextproc = NULL;
	return newProc;
}

job* initJob(convs* conv) {
	job* newjob = NULL;
	newjob = malloc(sizeof(job));
	if (newjob == NULL) {
		printf("malloc initJob error");
		return NULL;
	}
	newjob->state = LINKED;
	newjob->gpid = 0;
	newjob->conv = copyconv(conv);
	if (newjob->conv == NULL) {
		clear(newjob);
		return NULL;
	}
	newjob->nextjob = NULL;
	newjob->prevjob = NULL;
	process* curProc = NULL;
	for (int i = 0; i < conv->cntcommands; i++) {
		if (i == 0) {
			curProc = initProc(&cmds[curcmd]);
			if (curProc == NULL) {
				clear(newjob);
				return NULL;
			}
			newjob->proc = curProc;
		}
		else {
			curProc->nextproc = initProc(&cmds[curcmd + i]);
			if (curProc->nextproc == NULL) {
				clear(newjob);
				return NULL;
			}
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
			if (curJob == NULL) {
				continue;
			}
			curJob->prevjob = NULL;
			firstjob = curJob;
			lastjob = curJob;
		}
		else {
			curJob->nextjob = initJob(&conv[i]);
			if (curJob->nextjob == NULL) {
				continue;
			}
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