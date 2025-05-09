#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#define MAXARGS 256
#define MAXCMDS 50
#define MAXJOBS 20
#define BACKGROUND 0
#define FOREGROUND 1
struct command
{
    char *cmdargs[MAXARGS];
    int cmdflag;
    int bgk;
    char *infile;
    char *outfile;
    char *appfile;
};

/*  cmdflag's  */
#define OUTPIP 01
#define INPIP 02
extern struct command cmds[];
extern char bkgrnd;
// extern pid_t shell_pid;
// extern job_t *jobs;
// extern job_t *done_bg_jobs;
// extern string_node_t *new_done_jobs;
// extern int next_job_number;
// extern struct sigaction sa;
// extern struct sigaction blocked_signals;
// extern struct sigaction ignore;

int parseline(char *);
int promptline(char *, char *, int);