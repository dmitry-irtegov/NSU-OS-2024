#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct myStruct {
    char** arr;
    size_t cnt;
    size_t size;
    size_t num;
} myStruct;


void* func(void* paramPamPam) {
    myStruct* tctc = (myStruct*)paramPamPam;
    char** strings = tctc->arr;
    size_t cnt = tctc->cnt;
    size_t size = tctc->size;
    size_t num = tctc->num;
    size_t helper = (size / cnt) + ((size % cnt) != 0);

    for (int i = num * helper; i < ((num+1)*helper) && i < size; i++) {
        int j = num;
        while (j--) {
            printf("\t");
        }
        printf("%s\n", strings[i]);
    }
    pthread_exit(NULL);
}

#define CNT 4
myStruct meow[CNT];

int main() {
    pthread_t threads[CNT];

    char* str[] = {"a1", "a2", "a3", "a4", "a5", "a6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15",
                "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "b10", "b11", "b12",
                "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "c10", "c11", "c12",
                "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "aboba"};

    #define STRCNT (sizeof(str) / sizeof(char**))
    printf("%d\n\n\n", STRCNT);


    for (int i = 0; i < CNT; i++) {
            meow[i].arr  = str;
            meow[i].cnt  = CNT;
            meow[i].size = STRCNT;
            meow[i].num  = i;
        int thrErr = pthread_create(threads + i, NULL, func, meow + i);
        if (thrErr != 0) {
            char buf[256];
            strerror_r(thrErr, buf, sizeof(buf));
            fprintf(stderr, "Creating thread error: %d %s\n", i, &buf);
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

