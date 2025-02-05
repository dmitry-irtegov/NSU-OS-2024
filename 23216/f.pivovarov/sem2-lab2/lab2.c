#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *printTenStrings();
void errorHandler(int code, char *errorText);

int main() {
    int code;
    pthread_t thread;
    pthread_attr_t attr;

    if ((code = pthread_attr_init(&attr)) != 0) {
        errorHandler(code, "Attr initialization");
    }

    if ((code = pthread_create(&thread, &attr, printTenStrings, NULL)) != 0) {
        errorHandler(code, "Creating thread");
    }

    if ((code = pthread_join(thread, NULL)) != 0) {
        errorHandler(code, "Join");
    }

    if ((code = pthread_attr_destroy(&attr)) != 0) {
        errorHandler(code, "Attributes destroy");
    }

    printTenStrings();
}

void *printTenStrings() {
    for (int i = 1; i <= 10; i++) {
        printf("Print's number %d\n", i);
    }

    pthread_exit(NULL);
}

void errorHandler(int code, char *errorText) {
    char buf[256];
    strerror_r(code, buf, sizeof(buf));
    fprintf(stderr, "%s : %s\n", errorText, buf);
    exit(EXIT_FAILURE);
}
