#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINES 100
#define COEF (100*1000)

typedef struct MyStr {
    size_t len;
    char str[BUFSIZ];
} MyStr;

MyStr buffer[MAX_LINES];

void* kchau(void* arg) {
    int ind = (int)arg;

    usleep(COEF * buffer[ind].len);
    printf("%s\n", buffer[ind].str);
}

int main() {
    pthread_t threads[MAX_LINES];

    printf("Input up to 100 lines (Enter to stop):\n");

    int i = 0;
    for (i = 0; i < MAX_LINES; i++) {
        if (NULL == fgets(buffer[i].str, BUFSIZ, stdin)) {
            break;
        }

        size_t len = strlen(buffer[i].str);
        if ((len > 0) && (buffer[i].str[len - 1] == '\n')) {
            len--;
            buffer[i].str[len] = 0;
        }
        buffer[i].len = len;

        if (len == 0) {
            break;
        }
    }

    printf("\nSorted:\n");

    for (int j = 0; j < i; j++) {
        int thrErr = pthread_create(threads + j, NULL, kchau, (void*)j);
        if (thrErr != 0) {
            char buf[256];
            strerror_r(thrErr, buf, sizeof(buf));
            fprintf(stderr, "Creating thread error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], NULL);
    }

    exit(EXIT_SUCCESS);
}
