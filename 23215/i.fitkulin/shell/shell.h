#include <sys/types.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


#define MAXARGS 256
#define MAXCMDS 50

struct command {
    char *cmdargs[MAXARGS];
    char cmdflag;
};

typedef struct process
{
    struct process* next;       /* next process in pipeline */
    struct process* prev;       /* prev process in pipeline */
    struct command argv;        /* for exec */
    pid_t pid;                  /* process ID */
    int status;                 /* reported status value */
} process;

typedef struct job
{
    int number;
    struct job* next;           /* next active job */
    struct job* prev;           /* prev active job */
    char* command;              /* command line, used for messages */
    process* processes;         /* list of processes in this job */
    char notified;              /* true if user told about stopped job */
    pid_t pgid;                 /* process group ID */
    int status;
    int ground;
    int p_count; 
} job;

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

#define DONE    0
#define RUNNING 1
#define STOP    2

extern struct command cmds[];
extern char *infile, *outfile, *appfile;
extern char bkgrnd;
extern job* job_list;
extern int job_counter;

void init_shell();
int parseline(char *);
int promptline(char *, char *, int);
void start_process(process* p, int pipe1_fd[], int pipe2_fd[]);
void start_job(job* j);
void cd(char* arg);
void fg(char* arg);
void bg(char* arg);
void jobs();
job* create_job_from_cmds(char* command, struct command cmds[], int ncmds);
job* add_job(job *j);
job* find_job_by_number(int number);
void wait_for_job(job* j);    
int update_job_statuses();
void remove_job(job* j);
