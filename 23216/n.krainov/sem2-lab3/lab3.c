#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* print_strings(void* params){
    char** strings = (char**)params;
    for (int i = 0; i < 4; i++) {
        puts(strings[i]);
    }
    pthread_exit(NULL);
}

int init_strings(char** strings1, char** strings2, char** strings3, char** strings4) {
    strings1[0] = "first first";
    strings1[1] = "second first";
    strings1[2] = "third first";
    strings1[3] = "fourth first";

    strings2[0] = "first second";
    strings2[1] = "second second";
    strings2[2] = "third second";
    strings2[3] = "fourth second";

    strings3[0] = "first third";
    strings3[1] = "second third";
    strings3[2] = "third third";
    strings3[3] = "fourth third";

    strings4[0] = "first fourth";
    strings4[1] = "second fourth";
    strings4[2] = "third fourth";
    strings4[3] = "fourth fourth";

    return 0;
}

int main() {
    pthread_t thread1, thread2, thread3, thread4;

    char* strings1[4];
    char* strings2[4];
    char* strings3[4];
    char* strings4[4];

    init_strings(strings1, strings2, strings3, strings4);

    int code1, code2, code3, code4;
    code1 = pthread_create(&thread1, NULL, print_strings, strings1);
    code2 = pthread_create(&thread2, NULL, print_strings, strings2);
    code3 = pthread_create(&thread3, NULL, print_strings, strings3);
    code4 = pthread_create(&thread4, NULL, print_strings, strings4);

    if (code1 || code2 || code3 || code4) {
        fprintf(stderr, "some threads weren't create\n");
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}