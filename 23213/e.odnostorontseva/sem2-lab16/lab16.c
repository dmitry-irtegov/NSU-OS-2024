#include <stdio.h>
#include <semaphore.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main() {
    sem_t* parent_semaphore;
    sem_t* child_semaphore;

    parent_semaphore = sem_open("/parent_sem", O_CREAT | O_EXCL, 0644, 1);
    if (parent_semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    child_semaphore = sem_open("/child_sem", O_CREAT | O_EXCL, 0644, 0);
    if (child_semaphore == SEM_FAILED) {
        perror("sem_open");
        if(sem_unlink("/parent_sem") == -1) {
            perror("sem_unlink");
            exit(1);
        };
        exit(1);
    }

    pid_t pid;
    pid = fork();

    if (pid == -1) {
        perror("fork");
        if(sem_unlink("/parent_sem") == -1) {
            perror("sem_unlink");
            exit(1);
        };
        if(sem_unlink("/child_sem") == -1) {
            perror("sem_unlink");
            exit(1);
        };
        exit(1);
    } else if (pid == 0) {
        for (int i = 0; i < 10; i++) {
            if (sem_wait(child_semaphore) == -1) {
                perror("sem_wait");
                exit(1);
            }
            printf("Child: string of text\n");
            if (sem_post(parent_semaphore) == -1) {
                perror("sem_post");
                exit(1);
            }
        }
        exit(0);
    } else {
        for(int i = 0; i < 10; i++) {
            if (sem_wait(parent_semaphore) == -1) {
                perror("sem_wait");
                exit(1);
            }
            printf("Parent: string of text\n");
            if (sem_post(child_semaphore) == -1) {
                perror("sem_post");
                exit(1);
            }
        }
        
        if (sem_unlink("/parent_sem") == -1) {
            perror("sem_unlink");
            exit(1);
        }

        if (sem_unlink("/child_sem") == -1) {
            perror("sem_unlink");
            exit(1);
        }

        if(wait(NULL) == -1) {
            perror("wait");
            exit(1);
        }
        
        exit(0);
    }
}

