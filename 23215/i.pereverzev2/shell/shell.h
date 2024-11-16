#define MAXARGS 256
#define MAXCMDS 50
#include <sys/types.h>

struct command {
    char *cmdargs[MAXARGS];
    char cmdflag;
};

struct job {
    pid_t pgid;
    pid_t lidpid;
    char fgrnd;
    char stopped;
};

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

extern struct command cmds[];
extern char *infile, *outfile, *appfile;
extern char bkgrnd;

int parseline(char *);
int promptline(char *, char *, int);
