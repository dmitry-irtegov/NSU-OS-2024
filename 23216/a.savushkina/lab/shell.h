#define MAXARGS 256
#define MAXCMDS 50
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
int parseline(char *);
int promptline(char *, char *, int);