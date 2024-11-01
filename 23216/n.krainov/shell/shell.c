#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include "shell.h"

Job* head = NULL;

char* getPrompt(char* prompt){
    sprintf(prompt,"%s$ ", getcwd(NULL, 500));
    return prompt;
}

int createProcess(int in, int out, Command* cmd) {
    if (in != -1) {
        dup2(in, STDIN_FILENO);
        close(in);
    } 
    else if (cmd->infile != NULL) {
        int newIn = open(cmd->infile, O_RDONLY);
        dup2(newIn, STDIN_FILENO);
        close(newIn);
    }

    if (out != -1) {
        dup2(out, STDIN_FILENO);
        close(out);
    } 
    else if (cmd->outfile != NULL) {
        int newOut;
        if (cmd->flags & 1) {
            newOut = open(cmd->infile, O_WRONLY);    
        }
        else {
            newOut = open(cmd->outfile, O_WRONLY | O_APPEND);
        }
        
        dup2(newOut, STDOUT_FILENO);
        close(newOut);
    }
    execvp(cmd->cmdargs[0], cmd->cmdargs);
    return -1;
}

int createJob(Conv* conv) {
    Job* tail, *prev;
    if (head == NULL) {
        head = calloc(1, sizeof(Job));
    }
    else {
        tail = head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        prev = tail;
        tail->next = calloc(1, sizeof(Job));
        tail = tail->next;
    }


    int filedes[2];
    filedes[0] = -1;
    filedes[1] = -1;
    int in = -1, out = -1;
    for (Command* cmd = conv->cmd; cmd; cmd = cmd->next) {
        if (cmd->next) {
            puts("pipe");
            pipe(filedes);
            out = filedes[1];
        }

        pid_t pid = fork();

        switch (pid) {
            case 0:
                createProcess(in, out, cmd);
                break;
            default:

                break;
        }
        in = filedes[0];
        out = -1;
    }
    
    int status;
    waitpid(-1, &status, WUNTRACED);
    return 0;
}

int initShellPreferences() {
    if (!isatty(STDIN_FILENO)) {
        return -1;
    }

    sigset(SIGINT, SIG_IGN);
    sigset(SIGTSTP, SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);

    pid_t pid = getpid();
    if (setpgid(pid, pid) < 0) {
        return 1;
    }

    if (tcsetpgrp(STDIN_FILENO, pid)) {
        return 1;
    }
    return 0;
}

int main() {
    char line[1024];      /*  allow large command lines  */
    char prompt[500];      /* shell prompt */

    if (initShellPreferences()) {
        perror("initShellPreferences failed");
        exit(EXIT_FAILURE);
    }

    /* Conv* conv = NULL;
    while (promptline(getPrompt(prompt), line, sizeof(line)) > 0) {    
        conv = calloc(1, sizeof(Conv));
        if (parseline(line, conv) == -1) {
            continue;
        }
        else {
            Conv* h = conv;
            while (h != NULL) {
                Command* c = h->cmd;
                while (c != NULL) {
                    puts("args");
                    for (int i = 0; i < c->count_args; i++) {
                        printf("%s ", c->cmdargs[i]);
                    }
                    if (c->infile != NULL) {
                        puts("infile");
                        printf("%s", c->infile);
                    }
                    
                    if (c->infile != NULL) {
                        puts("outfile");
                        printf("%s", c->outfile);
                    }
                    c = c->next;
                }
                puts(" ");
                h = h->next;
            }
        }
    } */
    
   
    Conv* conv = NULL;
    
    while (promptline(getPrompt(prompt), line, sizeof(line)) > 0) {    
        conv = calloc(1, sizeof(Conv));
        if (parseline(line, conv) == -1) continue;   
        createJob(conv);
    }  
}