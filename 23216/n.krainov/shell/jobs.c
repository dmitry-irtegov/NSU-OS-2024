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

void createProcess(int in, int out, Command* cmd, pid_t pgid) {
    pid_t pid = getpid();
    if (pgid == 0) {
        if (setpgid(pid, pid)) {
            perror("createProcess");
            exit(EXIT_FAILURE);
        }
    }
    else {
        if (setpgid(pid, pgid)) {
            perror("createProcess");
            exit(EXIT_FAILURE);
        }
    }

    if (in != -1) {
        if (dup2(in, STDIN_FILENO)) {
            perror("createProcess");
            exit(EXIT_FAILURE);
        }
        close(in);
    } 
    else if (cmd->infile != NULL) {
        int newIn = open(cmd->infile, O_RDONLY);
        if (newIn == -1 || dup2(newIn, STDIN_FILENO) == -1) {
            perror("createProcess");
            exit(EXIT_FAILURE);
        }
        close(newIn);
    }

    if (out != -1) {
        if (dup2(out, STDOUT_FILENO) == -1) {
            perror("createProcess");
            exit(EXIT_FAILURE);
        }
        close(out);
    } 
    else if (cmd->outfile != NULL) {
        int newOut;
        if (cmd->flags & APP) {
            newOut = open(cmd->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else {
            newOut = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        
        if (newOut == -1 || dup2(newOut, STDOUT_FILENO) == -1) {
            perror("createProcess");
            exit(EXIT_FAILURE);
        }
        close(newOut);
    }

    sigset(SIGINT, SIG_DFL);
    sigset(SIGTSTP, SIG_DFL);
    sigset(SIGQUIT, SIG_DFL);
    sigset(SIGTTOU, SIG_DFL);
    sigset(SIGTTIN, SIG_DFL);

    switch (cmd->isShellSpecific) {
        case BG:
            bg(cmd);
            break;
        case JOBS:
            jobs();
            break;
        case FG:
            fg(cmd);
            break;
    }
    execvp(cmd->cmdargs[0], cmd->cmdargs);
    perror("exec");
    exit(EXIT_FAILURE);
}

void waitJob(Job* j) {
    int status;
    pid_t pid;
    do {
        pid = waitpid(-j->pgid, &status, WUNTRACED);
    } while (!updateInfoPid(j, pid, status) && !isStoppedJob(j) && !isCompletedJob(j));
}

int sendSIGCONT(pid_t pgid) {
    return kill(-pgid, SIGCONT);
}

int foregroundJob(Job* j, int continueJob) {
    if (tcsetpgrp(STDIN_FILENO, j->pgid)) {
        return -1;
    }
    
    if (continueJob) {
        if (sendSIGCONT(j->pgid)) {
            return -1;
        }
    }

    waitJob(j);
    if (tcsetpgrp(STDIN_FILENO, getpgrp())) {
        return -1;
    }

    if (isCompletedJob(j)) {
        freeJob(j);
    }
    return 0;
}

void createJobs(Conv* conv) {
    Job* tail;
    Process* curproc = NULL;
    for (Conv* curconv = conv; curconv; curconv = curconv->next) {
        if (head == NULL) {
            head = calloc(1, sizeof(Job));
            if (head == NULL) {
                perror("createJobs failed");
                return;
            }   
            head->number = 1;
            tail = head;
        }
        else {
            tail = head;
            while (tail->next != NULL) {
                tail = tail->next;
            }
            tail->next = calloc(1, sizeof(Job));
            if (tail->next == NULL) {
                perror("createJobs failed");
                return;
            }
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

            pid_t pid = fork();

            switch (pid) {
                case -1:
                    perror("createJobs failed");
                    return;
                case 0:
                    createProcess(in, out, cmd, tail->pgid);
                    break;
                default:
                    if (tail->pgid == 0) {
                        tail->pgid = pid;
                    }
                    if (tail->p == NULL) {
                        tail->p = calloc(1, sizeof(Process));
                        if (tail->p == NULL) {
                            perror("createJobs failed");
                            return;
                        }
                        tail->p->pid = pid;
                        curproc = tail->p;
                    }
                    else {
                        curproc->next = calloc(1, sizeof(Process));
                        if (curproc->next == NULL) {
                            perror("createJobs failed");
                            return;
                        }
                        curproc = curproc->next;
                        curproc->pid = pid;
                    }
                    if (setpgid(pid, tail->pgid)) {
                        perror("createJobs failed");
                        return;
                    }

                    for (int i = 0; i < cmd->count_args; i++) {
                        curproc->cmdargs[i] = calloc(strlen(cmd->cmdargs[i]) + 1, sizeof(char));
                        if (curproc->cmdargs[i] == NULL) {
                            perror("createJobs failed");
                            return;
                        }
                        strcpy(curproc->cmdargs[i], cmd->cmdargs[i]);
                    }
                    break;
            }
            if (in != -1 && close(in) == -1) {
                perror(NULL);
            }
            in = filedes[0];
            if (out != -1 && close(out) == -1) {
                perror(NULL);
            }
            
            out = -1;
        }
        
        if (!curconv->bg) {
            foregroundJob(tail, 0);
        }
    }
}