#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>  
#include <sys/wait.h>

char* child_sem =  "/child_sem";
char* parent_sem = "/parent_sem";


int main() {
    sem_t *sem_p = sem_open(parent_sem, O_CREAT | O_EXCL, 0644, 1);
    if (sem_p == SEM_FAILED) {
        perror("P: sem_open() fail");
        exit(EXIT_FAILURE);
    }

    sem_t *sem_c = sem_open(child_sem, O_CREAT | O_EXCL, 0644, 0);
    if (sem_c == SEM_FAILED) {
        sem_unlink(parent_sem);
        perror("C: sem_open() fail");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch (pid)
    {
        case 0:
            for (int i = 0; i < 10; i++) {
                if (sem_wait(sem_c) == -1) {
                    perror("sem_wait() fail");
                    exit(EXIT_FAILURE);
                }
                
                printf("child\n");

                if (sem_post(sem_p) == -1) {
                    perror("sem_post() fail");
                    exit(EXIT_FAILURE);
                }
            }
            exit(EXIT_SUCCESS);
            break;
        case -1:
            sem_unlink(child_sem);
            sem_unlink(parent_sem);
            perror("fork() fail");
            exit(EXIT_FAILURE);
            break;
        default:
            for (int i = 0; i < 10; i++) {
                if (sem_wait(sem_p) == -1) {
                    perror("sem_wait() fail");
                    exit(EXIT_FAILURE);
                }
                
                printf("parent\n");

                if (sem_post(sem_c) == -1) {
                    perror("sem_post() fail");
                    exit(EXIT_FAILURE);
                }
            }

            if (wait(NULL) == -1) {
                perror("wait() fail");
                exit(EXIT_FAILURE);
            }
            
            sem_unlink(child_sem);
            sem_unlink(parent_sem);
            exit(EXIT_SUCCESS);
    }
}
