#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

sem_t child;
sem_t parent;

void* thread_body(void *param){
    for(int i = 0;i<10;i++)
    {
        if(sem_wait(&child)!=0){
            perror("sem_wait");
            exit(1);
        }
        printf("Child\n");
        if(sem_post(&parent)!=0){
            perror("sem_post");
            exit(2);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]){
    if(sem_init(&parent, 0, 0)!=0){
        perror("sem_init");
        exit(3);
    }
    if(sem_init(&child, 0, 0)!=0){
        perror("sem_init");
        exit(3);
    }
    pthread_t thread;
    int code = pthread_create(&thread, NULL, thread_body, NULL);
    if(code!=0){
        perror("pthread_create");
        exit(4);
    }
    for(int i = 0;i<10;i++){
        printf("Parent\n");
        if(sem_post(&child)!=0){
            perror("sem_post");
            exit(2);
        }
        if(sem_wait(&parent)!=0){
            perror("sem_wait");
            exit(1);
        }
    }
    pthread_exit(NULL);
}