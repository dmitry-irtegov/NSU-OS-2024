#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include "shell.h"

extern Job* head;

int isStoppedJob(Job* j) {
    for (Process* p = j->p; p; p = p->next) {
        if (p->state == RUNNING) {
            return 0;
        }
    }

    return 1;
}

int isCompletedJob(Job* j) {
    for (Process* p = j->p; p; p = p->next) {
        if (p->state != DONE) {
            return 0;
        }
    }

    return 1;
}

//находим pid в одном из jobs и обновляем его статус
int updateInfoPid(Job* j, pid_t pid, int status) {
    if (pid == 0 || errno == ECHILD) {
        errno = 0;
        return -1;
    }
    else if (pid < 0) {
        perror("waitpid");
        return -1;
    }
    else {
        Job* start = (j == NULL) ? head : j;
        for (Job* cur = start; cur; cur = cur->next) {
            for (Process* p = cur->p; p; p = p->next) {
                if (p->pid == pid) {
                    p->status = status;
                    if (WIFSTOPPED(status))
                        p->state = STOP;
                    else {
                        p->state = DONE;
                    }
                    return 0;
                }
            }
        }
    }
    

    return -1;
}

void freeProcessList(Process* p) {
    if (p != NULL) {
        freeProcessList(p->next);
        free(p);
    }
}

void freeJob(Job* j) {
    if (j->prev != NULL && j->next != NULL) {
        j->prev->next = j->next;
        j->next->prev = j->prev;
    }
    else if (j->prev != NULL) {
        j->prev->next = NULL;
    }
    else if (j->next != NULL) {
        head = j->next;
        j->next->prev = NULL;
    }
    else { 
        head = NULL;
    }
    freeProcessList(j->p);
    free(j);
}

//обновляем информацию о jobs, удаляем завершившиеся job'ы. Если printinfo, выводим всю
//информацию о jobs, иначе выводим только обновившуюся информацию
void updateInfoJobs(int printInfo) {
    int status;
    pid_t pid;
    do {
        pid = waitpid(-1, &status, WUNTRACED|WNOHANG);
    } while (!updateInfoPid(NULL, pid, status));

    for (Job* j = head, *jnext; j; j = jnext) {
        if (printInfo) {
            printf("[%d] ", j->number);
            if (isCompletedJob(j)) {
                printf("Done\n");
            }
            else if (isStoppedJob(j)) {
                printf("Stopped\n");
            }
            else {
                printf("Running\n");
            }
        }

        if (isCompletedJob(j)) {
            if (!printInfo) {
                fprintf(stderr, "[%d] Done\n", j->number);
            }
            if (j->next != NULL) {
                jnext = j->next;
            }
            else {
                jnext = NULL;
            }

            freeJob(j);
            continue;
        }

        if (isStoppedJob(j) && j->notified == 0) {
            j->notified = 1;
            if (printInfo == 0) {
                fprintf(stderr, "[%d] Stop\n", j->number);
            }
        }

        jnext = j->next;
    }

} 