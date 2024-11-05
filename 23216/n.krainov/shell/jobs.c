#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include "shell.h"

Job* head = NULL;

int createProcess(int in, int out, Command* cmd, pid_t pgid) {
    pid_t pid = getpid();
    if (pgid == 0) {
        setpgid(pid, pid);
    }
    else {
        setpgid(pid, pgid);
    }

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
        dup2(out, STDOUT_FILENO);
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

    sigset(SIGINT, SIG_DFL);
    sigset(SIGTSTP, SIG_DFL);
    sigset(SIGQUIT, SIG_DFL);
    sigset(SIGTTOU, SIG_DFL);
    sigset(SIGTTIN, SIG_DFL);

    execvp(cmd->cmdargs[0], cmd->cmdargs);
    perror("exec");
    exit(EXIT_FAILURE);
}

int waitJob(Job* j) {
    int status;
    pid_t pid;
    do {
        pid = waitpid(-j->pgid, &status, WUNTRACED);
    } while (!updateInfoPid(j, pid, status) && !isStoppedJob(j) && !isCompletedJob(j));
}

int sendSIGCONT(pid_t pgid) {
    kill(-pgid, SIGCONT);
    return 0;
}

int foregroundJob(Job* j, int continueJob) {
    tcsetpgrp(STDIN_FILENO, j->pgid);
    
    if (continueJob) {
        sendSIGCONT(j->pgid);
    }

    waitJob(j);
    tcsetpgrp(STDIN_FILENO, getpgrp());

    if (isCompletedJob(j)) {
        freeJob(j);
    }
}

int createJobs(Conv* conv) {
    Job* tail;
    Process* curproc = NULL;
    for (Conv* curconv = conv; curconv; curconv = curconv->next) {
        if (head == NULL) {
            head = calloc(1, sizeof(Job));
            head->number = 1;
            tail = head;
        }
        else {
            tail = head;
            while (tail->next != NULL) {
                tail = tail->next;
            }
            tail->next = calloc(1, sizeof(Job));
            tail->next->number = tail->number+1;
            tail->next->prev = tail;
            tail = tail->next;
        }


        int filedes[2];
        filedes[0] = -1;
        filedes[1] = -1;
        int in = -1, out = -1;
        for (Command* cmd = curconv->cmd; cmd; cmd = cmd->next) {
            if (cmd->next) {
                pipe(filedes);
                out = filedes[1];
            }

            if (cmd->isShellSpecific) {
                switch (cmd->isShellSpecific) {
                    case FG:
                        fg(cmd);
                        break;
                    case BG:
                        bg(cmd);
                        break;
                    case JOBS:
                        jobs();
                        break;
                }
                continue;
            }

            pid_t pid = fork();

            switch (pid) {
                case -1:
                    break;
                case 0:
                    createProcess(in, out, cmd, tail->pgid);
                    break;
                default:
                    if (tail->pgid == 0) {
                        tail->pgid = pid;
                    }
                    if (tail->p == NULL) {
                        tail->p = calloc(1, sizeof(Process));
                        tail->p->pid = pid;
                        curproc = tail->p;
                    }
                    else {
                        curproc->next = calloc(1, sizeof(Process));
                        curproc = curproc->next;
                        curproc->pid = pid;
                    }
                    setpgid(pid, tail->pgid);
                    break;
            }
            close(in);
            in = filedes[0];
            close(out);
            out = -1;
        }
        
        if (!curconv->bg) {
            foregroundJob(tail, 0);
        }
    }
    
    return 0;
}