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
        if (j->number == num) {
            return j;
        }
    }

    return NULL;
}

void fg(Command* cmd) {
    if (cmd->next != NULL) {
        fprintf(stderr, "fg: no job control\n");
        exit(EXIT_FAILURE);
    }

    if (cmd->cmdargs[1] == NULL) {
        fprintf(stderr, "fg: Incorrect arg\n");
        exit(EXIT_FAILURE);
    }

    int num = atoi(cmd->cmdargs[1]);
    if (errno != 0) {
        fprintf(stderr, "fg: Incorrect arg\n");
        exit(EXIT_FAILURE);
    }

    Job* j = findJob(num);
    if (j == NULL) {
        fprintf(stderr, "fg: No such job\n");
        exit(EXIT_FAILURE);
    }

    j->notified = 0;
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == STOP) {
            p->state = RUNNING;
        }
    }

    if (foregroundJob(j, 1)) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void bg(Command* cmd) {
    if (cmd->cmdargs[1] == NULL) {
        fprintf(stderr, "Incorrect arg\n");
        exit(EXIT_FAILURE);
    }

    int num = atoi(cmd->cmdargs[1]);
    if (errno != 0) {
        fprintf(stderr, "Incorrect arg\n");
        exit(EXIT_FAILURE);
    }

    Job* j = findJob(num);
    if (j == NULL) {
        fprintf(stderr, "No such job\n");
        exit(EXIT_FAILURE);
    }

    j->notified = 0;
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == STOP) {
            p->state = RUNNING;
        }
    }

    if (sendSIGCONT(j->pgid)) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void jobs() {
    updateInfoJobs(1);
    exit(EXIT_SUCCESS);
}