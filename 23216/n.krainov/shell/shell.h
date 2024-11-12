#pragma once

#include <sys/types.h>


#define MAXARGS 256

#define RUNNING 0
#define DONE 1
#define STOP 2
#define KILLEDBYSIGNAL 3

typedef struct command{
    char *cmdargs[MAXARGS];
    int count_args;
    char* infile;
    char* outfile;
    char isShellSpecific; //for fg, bg and jobs
    char flags; //0000001 - >, 00000010 - >>
    struct command* next;
    struct command* prev;
}Command;

typedef struct conv {
    struct conv* next;
    Command* cmd;
    char bg;
}Conv;

typedef struct process{
    struct process* next;
    char *cmdargs[MAXARGS];
    pid_t pid;
    int state;
    int status;
}Process;

typedef struct job{
    int number;
    struct job* next;
    struct job* prev;
    Process* p;
    pid_t pgid;
    char notified;
} Job;

#define OUT  1
#define APP   2
#define APERSAND 4

#define FG 1
#define BG 2
#define JOBS 3

int parseline(char* line, Conv* task);
int promptline(char *line, int sizline);

void createJobs(Conv* conv);
void updateInfoJobs(int printInfo);

int isStoppedJob(Job* j);
int isCompletedJob(Job* j);

int updateInfoPid(Job* j, pid_t pid, int status);

int foregroundJob(Job* j, int continueJob);

int sendSIGCONT(pid_t pgid);

void fg(Command* cmd);
void bg(Command* cmd);
void jobs();

void freeJob(Job* j);

void freeSpace(Conv* task);