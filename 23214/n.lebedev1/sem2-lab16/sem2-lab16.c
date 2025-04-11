#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>

#define ITERATIONS 10
sem_t *sem1, *sem2;

int main() {
    pid_t pid;

    sem1 = sem_open("/sem1_lab16", O_CREAT, 0777, 1);
    if (sem1 == SEM_FAILED) {
        perror("ERROR: sem1 not created");
        exit(-1);
    }
    sem2 = sem_open("/sem2_lab16", O_CREAT, 0777, 0);
    if (sem2 == SEM_FAILED) {
        perror("ERROR: sem2 not created");
        if (sem_close(sem1) == -1) {
            perror("ERROR: sem1 not closed");
            exit(-1);
        }
        if (sem_unlink("/sem1_lab16") != 0) {
            perror("ERROR: sem1 not unlinked");
            exit(-1);
        }
        exit(-1);
    }

    pid = fork();
    if (pid == -1) {
        perror("ERROR: child process not created");
        if (sem_close(sem1) == -1) {
            perror("ERROR: sem1 not closed");
            exit(-1);
        }
        if (sem_close(sem2) == -1) {
            perror("ERROR: sem2 not closed");
            exit(-1);
        }

        if (sem_unlink("/sem1_lab16") != 0) {
            perror("ERROR: sem1 not unlinked");
            exit(-1);
        }
        if (sem_unlink("/sem2_lab16") != 0) {
            perror("ERROR: sem2 not unlinked");
            exit(-1);
        }
        exit(-1);
    }

    if (pid == 0) {
        for (int i = 0; i < ITERATIONS; i++) {
            sem_wait(sem2);
            printf("Child %d\n", i);
            sem_post(sem1);
        }
    } else {
        for (int i = 0; i < ITERATIONS; i++) {
            sem_wait(sem1);
            printf("Parent %d\n", i);
            sem_post(sem2);
        }
        if (wait(NULL) == -1) {
            perror("ERROR: can't wait the child process");
            if (sem_close(sem1) == -1) {
                perror("ERROR: sem1 not closed");
                exit(-1);
            }
            if (sem_close(sem2) == -1) {
                perror("ERROR: sem2 not closed");
                exit(-1);
            }

            if (sem_unlink("/sem1_lab16") != 0) {
                perror("ERROR: sem1 not unlinked");
                exit(-1);
            }
            if (sem_unlink("/sem2_lab16") != 0) {
                perror("ERROR: sem2 not unlinked");
                exit(-1);
            }
            exit(-1);
        }

        if (sem_close(sem1) == -1) {
            perror("ERROR: sem1 not closed");
            exit(-1);
        }
        if (sem_close(sem2) == -1) {
            perror("ERROR: sem2 not closed");
            exit(-1);
        }

        if (sem_unlink("/sem1_lab16") != 0) {
            perror("ERROR: sem1 not unlinked");
            exit(-1);
        }
        if (sem_unlink("/sem2_lab16") != 0) {
            perror("ERROR: sem2 not unlinked");
            exit(-1);
        }
        return 0;
    }
}