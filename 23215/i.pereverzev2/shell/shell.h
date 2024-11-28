#ifndef SHELLHENTRY
#define SHELLHENTRY

#include <sys/types.h>
#include <termios.h>


#define MAXARGS 256
#define MAXCMDS 50
#define MAXLINELEN 1024

struct command {
    char *cmdargs[MAXARGS];
    char cmdflag;
};

enum jstatus {
    NONE,
    RUNNING,
    STOPPED,
    DONE,
    TERMINATED
};

struct job {
    pid_t pgid;
    pid_t lidpid;
    char fgrnd;
    enum jstatus stat;
    char cmdline[MAXLINELEN];
    int prevjob;
    int nextjob;
    int instoplist;
    struct termios jobattr;
    int cnt_running;
    int cnt_stopped;
    int cnt_ended;
    int cnt_process;
};

typedef struct jobs_s {
    struct job *arr;
    int arsz;
    int last_id;
    int plus_id;
    int mins_id;
} jobsinfo;

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

extern struct command cmds[];
extern char *infile, *outfile, *appfile;
extern char emptyline;
extern char bkgrnd;
extern jobsinfo jobs;
extern struct termios shell_tattr;

int parseline(char *);
int promptline(char *, char *, int);

int update_jobs();
void print_jobs(int);
void fg(char *);
void bg(char *);
void job_to_fg(int, int);
void ensure_joblist_size();
void init_jobs();
void stoplist_add(int);
#endif