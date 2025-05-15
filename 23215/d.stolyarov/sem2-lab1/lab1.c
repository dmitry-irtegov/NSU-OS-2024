#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
void * printLines(void *line){
    char *name = (char*) line;
    for(int i = 0; i < 10; i++){
        printf("%s - %d line\n", name, i);
    }
    pthread_exit(0);
}

int main(){
    pthread_t thread;
    if(pthread_create(&thread, NULL, printLines, "child") != 0){
        perror("Thread creating error");
        exit(1);
    }
    printLines("parent");
    pthread_exit(0);
}