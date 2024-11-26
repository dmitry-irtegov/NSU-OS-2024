#define MAXARGS 256
#define MAXCMDS 50
#define MAXLINELEN 1024
#include <sys/types.h>
#include <termios.h>

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

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

extern struct command cmds[];
extern char *infile, *outfile, *appfile;
extern char emptyline;
extern char bkgrnd;

int parseline(char *);
int promptline(char *, char *, int);
