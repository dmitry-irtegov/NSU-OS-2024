#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void * sleepsort(void *string){
    usleep(10000 * strlen(string));
    printf("%s", string);
    return 0;
}

void threads_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

int main(){
    char strings[100][31];
    int n;
    for(n = 0; n < 100; n++){
        char* res = fgets(strings[k], 30, stdin);
        if(res == NULL){
            break;
        }
    }
    pthread_t* threads = malloc(sizeof(pthread_t)*n);
    if (threads == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < n; i++){
        if((code = pthread_create(&threads[i], NULL, sleepsort, strings[i])) != 0){
            threads_perror("pthread_create failed", code);
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}