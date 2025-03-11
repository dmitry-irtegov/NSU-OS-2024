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
    
    char* s = line, flag = 0;
    Conv* cur_conv = conv;
    Command* cur_cmd = NULL;
    char flagCommand = 1;
    char flagQuote = 0;
    while (*s) {
        s = blankskip(s);
        if (*s == '\0') {
            break;
        }

        switch (*s) {
            case '&':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv->next);
                    return -1;
                }
                flag = 1;
                cur_cmd = NULL;
                cur_conv->bg = 1;
                *s++ = '\0';
                break;
            case ';':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv->next);
                    return -1;
                }

                cur_cmd = NULL;
                flag = 1;
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
                if (s == NULL) {
                    freeSpace(conv->next);
                    return -1;
                }
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

                if (*s == '\"') {
                    *s++ = '\0';
                    flagQuote = 1;
                }

                cur_cmd->outfile = s;
                if (flagQuote) {
                    s = strpbrk(s, "\"");
                }
                else{
                    s = strpbrk(s, delim);
                }
                
                if (s == NULL) {
                    freeSpace(conv->next);
                    return -1;
                }
                flagQuote = 0;
                if (isspace(*s) || *s == '\"') *s++ = '\0';
            
                break;
            case '|':
                if (cur_cmd == NULL) {
                    fprintf(stderr, "Syntax error! the command was skipped!");
                    freeSpace(conv->next);
                    return -1;
                }
                
                cur_cmd->next = calloc(1, sizeof(Command));
                if (cur_cmd->next == NULL) {
                    perror("calloc failed");
                    freeSpace(conv->next);
                    return -1;
                }
                cur_cmd->next->prev = cur_cmd;
                cur_cmd = cur_cmd->next;
                *s++ = '\0';
                break;
            case '\"':
                if (flag) {
                    cur_conv->next = calloc(1, sizeof(Conv));
                    if (cur_conv->next == NULL) {
                        perror("calloc failed");
                        freeSpace(conv->next);
                        return -1;
                    }
                    cur_conv = cur_conv->next;
                    cur_cmd = NULL;
                    flag = 0;
                    flagCommand = 1;
                }
                if (cur_cmd == NULL) {
                    cur_conv->cmd = calloc(1, sizeof(Command));
                    if (cur_conv->cmd == NULL) {
                        perror("calloc failed");
                        freeSpace(conv->next);
                        return -1;
                    }
                    cur_cmd = cur_conv->cmd;
                }
                if (strncmp("fg", s, 2) == 0) {
                    cur_cmd->isShellSpecific = FG;
                }
                else if (strncmp("bg", s, 2) == 0) {
                    cur_cmd->isShellSpecific = BG;
                }
                else if (strncmp("jobs", s, 4) == 0 && flagCommand) {
                    cur_cmd->isShellSpecific = JOBS;
                }

                
                cur_cmd->cmdargs[cur_cmd->count_args++] = ++s;
                cur_cmd->cmdargs[cur_cmd->count_args] = NULL;
                s = strpbrk(s, "\"");
                if (s == NULL) {
                    perror(NULL);
                    freeSpace(conv->next);
                    return -1;
                }
                if (isspace(*s) || *s == '\"') *s++ = '\0';
                flagCommand = 0;

                break;
            default:
                if (flag) {
                    cur_conv->next = calloc(1, sizeof(Conv));
                    if (cur_conv->next == NULL) {
                        perror("calloc failed");
                        freeSpace(conv->next);
                        return -1;
                    }
                    cur_conv = cur_conv->next;
                    cur_cmd = NULL;
                    flag = 0;
                    flagCommand = 1;
                }
                if (cur_cmd == NULL) {
                    cur_conv->cmd = calloc(1, sizeof(Command));
                    if (cur_conv->cmd == NULL) {
                        perror("calloc failed");
                        freeSpace(conv->next);
                        return -1;
                    }
                    cur_cmd = cur_conv->cmd;
                }
                
                if (strncmp("fg", s, 2) == 0) {
                    cur_cmd->isShellSpecific = FG;
                }
                else if (strncmp("bg", s, 2) == 0) {
                    cur_cmd->isShellSpecific = BG;
                }
                else if (strncmp("jobs", s, 4) == 0 && flagCommand) {
                    cur_cmd->isShellSpecific = JOBS;
                }

                
                cur_cmd->cmdargs[cur_cmd->count_args++] = s;
                cur_cmd->cmdargs[cur_cmd->count_args] = NULL;
                if (*s == '\"') {
                    s = strpbrk(s, "\"");
                }
                else {
                    s = strpbrk(s, delim);
                }
                
                if (s == NULL) {
                    fprintf(stderr, "parseline failed");
                    freeSpace(conv->next);
                    return -1;
                }
                if (isspace(*s)) *s++ = '\0';
                flagCommand = 0;
                break;
        }
    }

    return 0;
    
}