#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define ITERATIONS 10
#define SEM_PERM 0600

void sync_mechanism() {
    const char* sync_a = "/sync_alpha_3F9X";
    const char* sync_b = "/sync_beta_ZK8R";
    
    sem_t *primary_sync = sem_open(sync_a, O_CREAT | O_EXCL, SEM_PERM, 1);
    if(primary_sync == SEM_FAILED) {
        perror("sync init failed");
        exit(EXIT_FAILURE);
    }

    sem_t *secondary_sync = sem_open(sync_b, O_CREAT | O_EXCL, SEM_PERM, 0);
    if(secondary_sync == SEM_FAILED) {
        sem_unlink(sync_a);
        perror("sync init failed");
        exit(EXIT_FAILURE);
    }

    pid_t new_proc = fork();
    
    if(new_proc == 0) {
        for(int count = 0; count < ITERATIONS;) {
            sem_wait(secondary_sync);
            printf("child\n");
            sem_post(primary_sync);
            count++;
        }
        sem_close(secondary_sync);
        exit(0);
    } 
    else if(new_proc > 0) {
        for(int cycle = 0; cycle < ITERATIONS;) {
            sem_wait(primary_sync);
            printf("parent\n");
            sem_post(secondary_sync);
            cycle++;
        }
        
        waitpid(new_proc, NULL, 0);
        sem_unlink(sync_a);
        sem_unlink(sync_b);
        sem_close(primary_sync);
    } 
    else {
        sem_unlink(sync_a);
        sem_unlink(sync_b);
        perror("process creation error");
        exit(EXIT_FAILURE);
    }
}

int main() {
    sync_mechanism();
    return 0;
}