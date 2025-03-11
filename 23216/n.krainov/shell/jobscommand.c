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

extern Job* head;

Job* findJob(int num){
    for (Job* j = head; j; j = j->next) {
        if ((num == -1 && j->next == NULL) || j->number == num) {
            return j;
        }
    }

    return NULL;
}

void fg(Command* cmd) {
    if (cmd->next != NULL || cmd->prev != NULL) {
        fprintf(stderr, "fg: no job control\n");
        return;
    }

    int num;
    if (cmd->cmdargs[1] == NULL) {
        num = -1;
    }
    else {
        num = atoi(cmd->cmdargs[1]);
        if (errno != 0) {
            fprintf(stderr, "fg: Incorrect arg\n");
            return;
        }
    }

    Job* j = findJob(num);
    if (j == NULL) {
        fprintf(stderr, "fg: No such job\n");
        return;
    }

    j->notified = 0;
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == STOP) {
            p->state = RUNNING;
        }

        int index = 0;
        while (p->cmdargs[index] != NULL && index < MAXARGS) {
            fprintf(stderr, "%s ", p->cmdargs[index]);
            index++;
        }

        if (p->next != NULL) {
            fprintf(stderr, "| ");
        }
    }

    fprintf(stderr, "\n");
    if (foregroundJob(j, 1)) {
        return;
    }
}

void bg(Command* cmd) {
    if (cmd->next != NULL || cmd->prev != NULL) {
        fprintf(stderr, "bg: no job control\n");
        return;
    }

    int num;
    if (cmd->cmdargs[1] == NULL) {
        num = -1;
    }
    else {
        num = atoi(cmd->cmdargs[1]);
        if (errno != 0) {
            fprintf(stderr, "bg: Incorrect arg\n");
            return;
        }
    }

    Job* j = findJob(num);
    if (j == NULL) {
        fprintf(stderr, "bg: No such job\n");
        return;
    }

    j->notified = 0;
    fprintf(stderr, "[%d] ", j->number);
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == STOP) {
            p->state = RUNNING;
        }

        int index = 0;
        while (p->cmdargs[index] != NULL && index < MAXARGS) {
            fprintf(stderr, " %s", p->cmdargs[index]);
            index++;
        }

        if (p->next != NULL) {
            fprintf(stderr, " |");
        }
    }

    fprintf(stderr, "\n");
    if (sendSIGCONT(j->pgid)) {
        perror("bg");
        return;
    }
}

void jobs() {
    updateInfoJobs(1);
    exit(EXIT_SUCCESS);
}