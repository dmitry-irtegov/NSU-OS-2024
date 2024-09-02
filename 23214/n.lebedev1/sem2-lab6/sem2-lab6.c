#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN 101
#define MAX_STR_NUM 100
#define DELAY 50000

void *sleepsort(void *args) {
    char *str = (char *) args;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
        len--;
    }
    usleep(len * DELAY);
    printf("%s\n", str);
    return 0;
}

void free_buf(char **buf, int N) {
    for (int i = 0; i < N; i++) {
        free(buf[i]);
    }
    free(buf);
}

int main() {
    int errCode;
    pthread_t pthreads[MAX_STR_NUM];
    char **buf = (char **) malloc(sizeof(char *) * MAX_STR_NUM);
    if (!buf) {
        perror("ERROR: Allocation failed");
        exit(1);
    }
    int i, N;

    for (i = 0; i < MAX_STR_NUM; i++) {
        buf[i] = (char *) malloc(sizeof(char) * MAX_LEN);
        if (!buf) {
            free_buf(buf, i);
            perror("ERROR: Allocation failed");
            exit(1);
        }
        if (fgets(buf[i], MAX_LEN, stdin) == NULL) {
            free(buf[i]);
            break;
        }
    }
    N = i;

    printf("Sorted strings:\n");
    for (i = 0; i < N; i++) {
        if ((errCode = pthread_create(&pthreads[i], NULL, sleepsort, buf[i])) != 0) {
            free_buf(buf, N);
            fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
            exit(1);
        }
    }
    for (i = 0; i < N; i++) {
        pthread_join(pthreads[i], NULL);
    }
    free_buf(buf, N);
    return 0;
}
