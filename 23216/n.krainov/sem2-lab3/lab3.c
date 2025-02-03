#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* print_strings(void* params){
    char** strings = (char**)params;
    puts(strings[0]);
    puts(strings[1]);
    puts(strings[2]);
    puts(strings[3]);
    pthread_exit(NULL);
}

int init_strings(char** strings1, char** strings2, char** strings3, char** strings4) {
    strings1[0] = strdup("first first");
    strings1[1] = strdup("second first");
    strings1[2] = strdup("third first");
    strings1[3] = strdup("fourth first");

    if (strings1[0] == NULL ||
        strings1[1] == NULL || 
        strings1[2] == NULL ||
        strings1[3] == NULL) {
        
        return 1;
    }

    strings2[0] = strdup("first second");
    strings2[1] = strdup("second second");
    strings2[2] = strdup("third second");
    strings2[3] = strdup("fourth second");

    if (strings2[0] == NULL ||
        strings2[1] == NULL || 
        strings2[2] == NULL ||
        strings2[3] == NULL) {
        
        return 1;
    }

    strings3[0] = strdup("first third");
    strings3[1] = strdup("second third");
    strings3[2] = strdup("third third");
    strings3[3] = strdup("fourth third");

    if (strings3[0] == NULL ||
        strings3[1] == NULL || 
        strings3[2] == NULL ||
        strings3[3] == NULL) {
        
        return 1;
    }

    strings4[0] = strdup("first fourth");
    strings4[1] = strdup("second fourth");
    strings4[2] = strdup("third fourth");
    strings4[3] = strdup("fourth fourth");

    if (strings4[0] == NULL ||
        strings4[1] == NULL || 
        strings4[2] == NULL ||
        strings4[3] == NULL) {
        
        return 1;
    }

    return 0;
}

int main() {
    pthread_t thread1, thread2, thread3, thread4;

    char** strings1 = calloc(4, sizeof(char**));
    char** strings2 = calloc(4, sizeof(char**));
    char** strings3 = calloc(4, sizeof(char**));
    char** strings4 = calloc(4, sizeof(char**));

    if (strings1 == NULL ||
        strings2 == NULL ||
        strings3 == NULL ||
        strings4 == NULL) {
        exit(EXIT_FAILURE);
    }

    if (init_strings(strings1, strings2, strings3, strings4)) {
        exit(EXIT_FAILURE);
    }

    int code1, code2, code3, code4;
    code1 = pthread_create(&thread1, NULL, print_strings, strings1);
    code2 = pthread_create(&thread2, NULL, print_strings, strings2);
    code3 = pthread_create(&thread3, NULL, print_strings, strings3);
    code4 = pthread_create(&thread4, NULL, print_strings, strings4);

    if (code1 || code2 || code3 || code4) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}