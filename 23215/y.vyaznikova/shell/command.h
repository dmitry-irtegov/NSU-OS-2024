#ifndef COMMAND_H
#define COMMAND_H

#define MAXARGS 256

struct command {
    char *cmdargs[MAXARGS];
    char cmdflag;
};

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

#endif /* COMMAND_H */
