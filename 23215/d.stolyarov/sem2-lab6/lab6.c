#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
int coef;
char ar[101][11];

void * sleepsort(void *line){
    sleep(coef * strlen(line));
    printf("%s", line);
    return 0;
}

int main(){
    coef = 1;
    pthread_t thread;
    int k;
    for(k = 0; k < 100 && fgets(ar[k], 10, stdin); k++);
    printf("\n________\n");
    for(int i = 0; i < k; i++){
        if(pthread_create(&thread, NULL, sleepsort, ar[i]) != 0){
            perror("Thread creating error");
            exit(1);
        }
    }
    pthread_exit(0);
}