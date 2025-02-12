#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>

char* parent = "/parent";
char* child =  "/child";

void my_wait(sem_t* my_sem) {
    if (sem_wait(my_sem) == -1) {
        perror("sem_wait fail");
        exit(-1);
    }
}

void my_post(sem_t* my_sem) {
    if (sem_post(my_sem) == -1) {
        perror("sem_post fail");
        exit(-1);
    }
}

int main(int argc, char** argv) {
    sem_t* sem_parent = sem_open(parent, O_CREAT | O_EXCL, 0644, 1);
    if (sem_parent == SEM_FAILED) {
        perror("sem_open fail");
        exit(-1);
    }
    
    sem_t* sem_child = sem_open(child, O_CREAT | O_EXCL, 0644, 0);
    if (sem_child == SEM_FAILED) {
        sem_unlink(parent);
        perror("sem_open fail");
        exit(-1);
    }

    pid_t pid;
    pid = fork();
    switch (pid)
    {
    case -1:
        sem_unlink(child);
        sem_unlink(parent);
        perror("fork failed");
        exit(-1);
    case 0:
        for (int i = 0; i < 10; i++) {
            my_wait(sem_child);
            printf("child\n");
            my_post(sem_parent);
        }
        exit(0);
    default:
        sem_unlink(child);
        sem_unlink(parent);
        for (int i = 0; i < 10; i++) {
            my_wait(sem_parent);
            printf("parent\n");
            my_post(sem_child);
        }
        if(wait(NULL) == -1) {
            perror("wait failed");
            exit(-1);
        }
        exit(0);
    }
}
