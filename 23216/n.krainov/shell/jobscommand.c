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

extern Job* head;

Job* findJob(int num){
    for (Job* j = head; j; j = j->next) {
        if (j->number == num) {
            return j;
        }
    }

    return NULL;
}

int fg(Command* cmd) {
    if (cmd->next != NULL) {
        fprintf(stderr, "fg: no job control\n");
        return -1;
    }

    if (cmd->cmdargs[1] == NULL) {
        fprintf(stderr, "fg: Incorrect arg\n");
        return -1;
    }

    int num = atoi(cmd->cmdargs[1]);
    if (errno != 0) {
        fprintf(stderr, "fg: Incorrect arg\n");
        return -1;
    }

    Job* j = findJob(num);
    if (j == NULL) {
        fprintf(stderr, "fg: No such job\n");
        return -1;
    }

    j->notified = 0;
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == STOP) {
            p->state = RUNNING;
        }
    }

    if (foregroundJob(j, 1)) {
        return -1;
    }

    return 0;
}

int bg(Command* cmd) {
    if (cmd->cmdargs[1] == NULL) {
        fprintf(stderr, "Incorrect arg\n");
        return -1;
    }

    int num = atoi(cmd->cmdargs[1]);
    if (errno != 0) {
        fprintf(stderr, "Incorrect arg\n");
        return -1;
    }

    Job* j = findJob(num);
    if (j == NULL) {
        fprintf(stderr, "No such job\n");
        return -1;
    }

    j->notified = 0;
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == STOP) {
            p->state = RUNNING;
        }
    }

    if (sendSIGCONT(j->pgid)) {
        return -1;
    }

    return 0;
}

void jobs() {
    updateInfoJobs(1);
}