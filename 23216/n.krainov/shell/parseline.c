#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"

char* blankskip(char *s) {
    while (isspace(*s) && *s) ++s;
    return s;
}

void freeSpace(Conv* task) {
    if (task != NULL) {
        freeSpace(task->next);
        free(task);
    }
}

int parseline(char *line, Conv* conv) {
    char delim[] = "\t|&<>;\n ";
    
    char* s = line;
    Conv* cur_conv = conv;
    Command* cur_cmd = NULL;
    while (*s) {
        s = blankskip(s);
        if (*s == '\0') {
            break;
        }

        switch (*s) {
            case '&':
                cur_conv->bg = 1;
                cur_conv->next = calloc(1, sizeof(Conv));
                cur_conv = cur_conv->next;
                cur_cmd = NULL;
                *s++ = '\0';
                break;
            case ';':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv->next);
                    return -1;
                }

                cur_conv->next = calloc(1, sizeof(Conv));
                cur_conv = cur_conv->next;
                cur_cmd = NULL;
                *s++ = '\0';
                break;
            case '<':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv);
                    return -1;
                }

                *s++ = '\0';
                s = blankskip(s);

                cur_cmd->infile = s;
                s = strpbrk(s, delim);
                if (isspace(*s)) *s++ = '\0';

                break;
            case '>':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv->next);
                    return -1;
                }
                if (*(s+1) == '>') { 
                    cur_cmd->flags |= APP;
                }
                *s++ = '\0';
                
                s = blankskip(s);

                cur_cmd->outfile = s;
                s = strpbrk(s, delim);
                if (isspace(*s)) *s++ = '\0';
                break;
            case '|':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv->next);
                    return -1;
                }

                cur_cmd->next = calloc(1, sizeof(Command));
                cur_cmd = cur_cmd->next;
                *s++ = '\0';
                break;
            default:
                if (cur_cmd == NULL) {
                    cur_conv->cmd = calloc(1, sizeof(Command));
                    cur_cmd = cur_conv->cmd;
                    cur_cmd->next = NULL;
                }
                cur_cmd->cmdargs[cur_cmd->count_args++] = s;
                cur_cmd->cmdargs[cur_cmd->count_args] = NULL;
                s = strpbrk(s, delim);
                if (isspace(*s)) *s++ = '\0';
                break;
        }
    }

    return 0;
    
}