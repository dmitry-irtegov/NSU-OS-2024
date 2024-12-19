#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"
static char* blankskip(register char*);

char* ss = NULL;
char* sss = NULL;


int parseline(char* line) {

    if (!sss)
        free(sss);
    ss = (char*)malloc(1024);
    sss = ss;
    int nargs, ncmds;
    register char* s;
    char aflg = 0;
    int rval;
    register int i;
    static char delim[] = " \t|&<>;\n";
    pid_t pid;
    memset(ss, 0, 1024); 


    /* Initialize */
    bkgrnd = nargs = ncmds = rval = 0;
    s = line;
    infile = outfile = appfile = (char*)NULL;
    cmds[0].cmdargs[0] = (char*)NULL;

    for (i = 0; i < MAXCMDS; i++) {
        cmds[i].cmdflag = 0;
    }

    while (*s) { /* Until line has been parsed */
        s = blankskip(s); /* Skip white space */
        if (!*s) break; /* Done with line */

        /* Handle <, >, |, &, ;, %, and jobs */
        switch (*s) {
        case '&':
            ++bkgrnd;
            *s++ = '\0';
            break;

        case '>':
            if (*(s + 1) == '>') {
                ++aflg;
                *s++ = '\0';
            }
            *s++ = '\0';
            s = blankskip(s);
            if (!*s) {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            if (aflg)
                appfile = s;
            else
                outfile = s;
            s = strpbrk(s, delim);
            if (isspace(*s))
                *s++ = '\0';
            break;

        case '<':
            *s++ = '\0';
            s = blankskip(s);
            if (!*s) {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            infile = s;
            s = strpbrk(s, delim);
            if (isspace(*s))
                *s++ = '\0';
            break;

        case '|':
            if (nargs == 0) {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            cmds[ncmds++].cmdflag |= OUTPIP;
            cmds[ncmds].cmdflag |= INPIP;
            *s++ = '\0';
            nargs = 0;
            break;

        case ';':
            *s++ = '\0';
            ++ncmds;
            nargs = 0;
            break;

        case '%':  // Special handling for jobs
            s++;
            if (*s == '\0') {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            if (*s == '+' || *s == '-' || isdigit(*s)) {
                pid = (isdigit(*s)) ? get_g_int(atoi(s)) : get_g_ch(*s);
                if (pid == 0) {
                    fprintf(stderr, "No such job\n");
                    return -1;
                }
            }
            else {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            sprintf(ss, "%d", pid);
            cmds[ncmds].cmdargs[nargs++] = ss;
            cmds[ncmds].cmdargs[nargs] = (char*)NULL;
            ss += strlen(ss) + 1;
            s = strpbrk(s, delim);
            if (s && isspace(*s)) {
                *s++ = '\0';
            }
            break;

        default:

            if (nargs == 0) /* Next command */
                rval = ncmds + 1;

            cmds[ncmds].cmdargs[nargs++] = s;
            cmds[ncmds].cmdargs[nargs] = (char*)NULL;

            s = strpbrk(s, delim);
            fprintf(stderr, "%c", *s);
            if (isspace(*s))
                *s++ = '\0';

            break;
        }
    }

    /* Error check */
    if (cmds[ncmds - 1].cmdflag & OUTPIP) {
        if (nargs == 0) {
            fprintf(stderr, "syntax error\n");
            return -1;
        }
    }

    return rval;
}


static char* blankskip(register char* s)
{
    while (isspace(*s) && *s) ++s;
    return(s);
}

void free_ss() {
    if(!sss)
        free(sss);
}