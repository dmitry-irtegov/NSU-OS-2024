#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "shell.h"

// struct command cmds[MAXCMDS];
static char *blankskip(register char *);

int parseline(char *line)
{
    int nargs, ncmds;
    char *s;
    char aflg = 0;
    int rval;
    static char delim[] = " \t|&<>;\"\'\n";
    ncmds = 0;
    s = line;

    for (int i = 0; i < MAXCMDS; i++)
    {
        cmds[i].cmdargs[0] = NULL;
        cmds[i].cmdargs[1] = NULL;
        cmds[i].cmdargs[2] = NULL;
        cmds[i].cmdflag = 0;
        cmds[i].bgk = 1;
        cmds[i].infile = NULL;
        cmds[i].outfile = NULL;
        cmds[i].appfile = NULL;
    }

    int current_cmd = 0;
    while (s != NULL)
    {
        s = blankskip(s);
        if (!*s)
            break;

        switch (*s)
        {
            
        case '"':
            s++; 
            char *start = s; 
            s = strpbrk(s, "\""); 
            if (s) {
                *s = '\0'; 
                cmds[current_cmd].cmdargs[nargs++] = start; 
                s++;
            }
            cmds[current_cmd].cmdargs[nargs] = NULL; 
        

            break;
        
        case '&':
            cmds[current_cmd].bgk = 0;
            cmds[current_cmd].cmdargs[nargs] = NULL;
            current_cmd++;
            *s++ = '\0';
            break;
        case '>':
            if (current_cmd >= MAXCMDS)
            {
                fprintf(stderr, "Too many commands\n");
                return -1;
            }
            aflg = (*s == '>' && *(s + 1) == '>');
            s += aflg ? 2 : 1;
            s = blankskip(s);
            if (!*s)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            if (cmds[current_cmd].cmdargs[0] == NULL)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            if (aflg)
            {
                if (cmds[current_cmd].appfile != NULL)
                {
                    fprintf(stderr, "syntax error: multiple append redirections\n");
                    return -1;
                }
                cmds[current_cmd].appfile = s;
            }
            else
            {
                if (cmds[current_cmd].outfile != NULL)
                {
                    fprintf(stderr, "syntax error: multiple output redirections\n");
                    return -1;
                }
                cmds[current_cmd].outfile = s;
            }
            s = strpbrk(s, delim);
            if (s && isspace(*s))
                *s++ = '\0';
        else{
            *s = '>';
            *s++ = '\0';
        }
            break;
        case '<':
            if (current_cmd >= MAXCMDS)
            {
                fprintf(stderr, "Too many commands\n");
                return -1;
            }
            if (cmds[current_cmd].cmdargs[0] == NULL)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            if (cmds[current_cmd].infile != NULL)
            {
                fprintf(stderr, "syntax error: multiple input redirections\n");
                return -1;
            }
            *s++ = '\0';
            s = blankskip(s);
            if (!*s)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            cmds[current_cmd].infile = s;
            s = strpbrk(s, delim);
            if (s && isspace(*s))
                *s++ = '\0';
            break;
        case '|':
            if (cmds[current_cmd].cmdargs[0] == NULL)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            cmds[current_cmd].cmdflag |= OUTPIP;
            current_cmd++;
            if (current_cmd >= MAXCMDS)
            {
                fprintf(stderr, "Too many commands\n");
                return -1;
            }
            cmds[current_cmd].cmdflag |= INPIP;
            *s++ = '\0';
            nargs = 0;
            break;
        case ';':
            if (cmds[current_cmd].cmdargs[0] == NULL)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            current_cmd++;
            if (current_cmd >= MAXCMDS)
            {
                fprintf(stderr, "Too many commands\n");
                return -1;
            }
            *s++ = '\0';
            nargs = 0;
            break;
        default:
            if (cmds[current_cmd].cmdargs[0] == NULL)
            {
                cmds[current_cmd].cmdargs[0] = s;
                nargs = 1;
                ncmds++;
            }
            else if (nargs < MAXARGS - 1)
            {
                if (*s){
                    cmds[current_cmd].cmdargs[nargs++] = s;
                }
                cmds[current_cmd].cmdargs[nargs] = NULL;
            }
            else
            {
                fprintf(stderr, "Too many arguments\n");
                return -1;
            }
            s = strpbrk(s, delim);
            if (s && isspace(*s))
                *s++ = '\0';
            break;
        }
    }

    // // Finalize the last command's arguments
   if (cmds[current_cmd].cmdargs[0] != NULL)
    {
        if (nargs != 0)
            cmds[current_cmd].cmdargs[nargs] = NULL;
        ncmds = current_cmd + 1;
    }
    else
    {
        if (cmds[current_cmd - 1].cmdargs[0] != NULL){
            if (nargs != 0)
            cmds[current_cmd - 1].cmdargs[nargs] = NULL;
        ncmds = current_cmd;
        }
        else{
        return -1;
        }
    }

    for (int i = 0; i < ncmds; i++)
    {
        if (cmds[i].cmdflag & OUTPIP)
        {
            if (i >= ncmds - 1)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
            if (!(cmds[i + 1].cmdflag & INPIP))
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
        }
        if (cmds[i].cmdflag & INPIP)
        {
            if (cmds[i].cmdargs[0] == NULL)
            {
                fprintf(stderr, "syntax error\n");
                return -1;
            }
        }
    }
    rval = ncmds;
    return rval;
}

static char *blankskip(register char *s)
{
    while (isspace(*s) && *s){
        ++s;
    }
    return (s);
}