#pragma once
#include <unistd.h>
#include "shell.h"
#include <termios.h>

typedef struct process {
	command* cmd;
	struct process* nextproc;
}process;

/*job`s state*/
#define RUNNING 0
#define STOPPED 1
#define LINKED 2

typedef struct job {
	convs* conv;
	process* proc;
	struct job* nextjob, * prevjob;
	char state;
	int number;
	pid_t gpid;
	struct termios terminalattr;
} job;