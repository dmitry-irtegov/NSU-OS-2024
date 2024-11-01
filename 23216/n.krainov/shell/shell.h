#pragma once

#include <sys/types.h>

#define MAXARGS 256

typedef struct command{
    char *cmdargs[MAXARGS];
    int count_args;
    char* infile;
    char* outfile;
    char flags; //0000001 - >, 00000010 - >>
    struct command* next;
}Command;

typedef struct conv {
    struct conv* next;
    Command* cmd;
    char bg;
}Conv;

typedef struct job{
    pid_t ppid;
    int status;
    struct job* next;
} Job;

typedef struct process{
    struct process* next;
    pid_t pid;
    int status;
}Process;

#define OUT  1
#define APP   2
#define APERSAND 4

int parseline(char* line, Conv* task);
int promptline(char *prompt, char *line, int sizline);