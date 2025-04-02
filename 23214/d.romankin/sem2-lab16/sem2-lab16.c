#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>           
#include <sys/stat.h>  
#include <unistd.h>
#include <wait.h>
sem_t* sem2;
sem_t* sem1;


void mainThread() {
	for (int i = 0; i < 10; i++) {
		assert(sem_wait(sem1) == 0);
		printf("Parent %d\n", i);
		assert(sem_post(sem2) == 0);		
	}


}

int main() {
    sem2 = sem_open("/sem2", O_CREAT, 0777, 0);
	if (sem2 == SEM_FAILED) {
		perror("semaphore 2 init error");
		exit(EXIT_FAILURE);
	}
	sem1 = sem_open("/sem1", O_CREAT, 0777, 1);
	if (sem1 == SEM_FAILED) {
		perror("semaphore 1 init error");
		exit(EXIT_FAILURE);
	}
    pid_t child_pid;
    if ((child_pid = fork()) == -1) {
        perror("fork failed");
        if (sem_close(sem1) != 0) {
            perror("semaphore 1 close error");
            exit(EXIT_FAILURE);
        }
        if (sem_close(sem2) != 0) {
            perror("semaphore 2 close error");
            exit(EXIT_FAILURE);
        }
        
        if (sem_unlink("/sem1") != 0) {
            perror("semaphore 1 unlink error");
            exit(EXIT_FAILURE);
        }
        if (sem_unlink("/sem2") != 0) {
            perror("semaphore 2 unlink error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    } 
    else {
        if (child_pid == 0) {
            for (int i = 0; i < 10; i++) {
                assert(sem_wait(sem2) == 0);
                printf("Child %d\n", i);
                assert(sem_post(sem1) == 0);
            }
        }
        else {
            mainThread();
            if (wait(NULL) == -1) {
                perror("wait error");
                if (sem_close(sem1) != 0) {
                    perror("semaphore 1 close error");
                    exit(EXIT_FAILURE);
                }
                if (sem_close(sem2) != 0) {
                    perror("semaphore 2 close error");
                    exit(EXIT_FAILURE);
                }
                
                if (sem_unlink("/sem1") != 0) {
                    perror("semaphore 1 unlink error");
                    exit(EXIT_FAILURE);
                }
                if (sem_unlink("/sem2") != 0) {
                    perror("semaphore 2 unlink error");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            }
            else {
                
                if (sem_close(sem1) != 0) {
                    perror("semaphore 1 close error");
                    exit(EXIT_FAILURE);
                }
                if (sem_close(sem2) != 0) {
                    perror("semaphore 2 close error");
                    exit(EXIT_FAILURE);
                }
                
                
                if (sem_unlink("/sem1") != 0) {
                    perror("semaphore 1 unlink error");
                    exit(EXIT_FAILURE);
                }
                if (sem_unlink("/sem2") != 0) {
                    perror("semaphore 2 unlink error");
                    exit(EXIT_FAILURE);
                }
                
                exit(EXIT_SUCCESS);
            }
        }
    }
    

	
}
