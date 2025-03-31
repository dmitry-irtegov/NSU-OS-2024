#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define SIZE 100
#define MAX_LENGTH 1000 
#define SLEEP_TIME 10000

char arr[SIZE][MAX_LENGTH];

void* sleep_sort(void *param) {
    usleep(SLEEP_TIME * strlen(arr[(intptr_t)param]));
    printf("%s\n", arr[(intptr_t)param]); 
    return NULL;  
}

int main() {
    int size = 0;

    for (int i = 0; i < SIZE; i++) {
        char *str = fgets(arr[i], sizeof(arr[i]), stdin);
        if (str == NULL) {
            size = i;
            break;
        }

        int len = strlen(arr[i]);
        if (len > 0 && arr[i][len - 1] == '\n') {
            arr[i][len - 1] = '\0';
            len--;
        }
        if (len == 0) {
            i--;
        }
    }

    printf("\n======RESULT=====\n");

    pthread_t threads[SIZE];

    for (int i = 0; i < size; i++) {
        pthread_create(&threads[i], NULL, sleep_sort, (void *)(intptr_t)i);
    }

    for (int i = 0; i < size; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}