#pragma once
#define MAXARGS 256
#define MAXCONV 256
#define MAXCMDS 50

typedef struct streams {
	char* file;
	char flags;
} streams;


typedef struct convs {
	unsigned char flag;
	unsigned char cntcommands;
	streams in, out, err;
} convs;

typedef struct command {
	char* cmdargs[MAXARGS];
	char cmdflag;
} command;


/* cmdflag`s */
#define OUTPIP 01
#define INPIP 02


/*streams`s flags*/
#define ISEXIST 1
#define ISCONT 2

/*type`s types*/
#define START 1
#define END 2

/*conv`s flags*/
#define BKGRND 1


extern struct command cmds[];
extern struct convs conv[];


int parseline(char*);
int promptline(char*, int);
void clearPars();