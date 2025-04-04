#define MAXARGS 256
#define MAXCMDS 50

struct command {
    char *cmdargs[MAXARGS];
    char cmdflag;
    char *infile, *outfile, *appfile;
    char bkgrnd;
};

////////////////////
typedef enum {
    BG,
    FG,
    JB
} shellCommands;

typedef struct {
    int argc;
    char *cmdargs[MAXARGS];
    char *infile, *outfile, *appfile;
    char bkgrnd;
    char cmdflag; 
    shellCommands shellCommandCode; 
    command* next, *prev;
} command;

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

extern command cmds[];

int parseline(char *);
int promptline(char *, char *, int);
