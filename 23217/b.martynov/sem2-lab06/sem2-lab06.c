#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINES 100
#define COEF (100*1000)

pthread_mutex_t mut;

typedef struct MyStr {
    size_t len;
    char str[BUFSIZ];
} MyStr;

MyStr arr[MAX_LINES];
MyStr sorted[MAX_LINES];
int ptr = 0;

void* sleepSortWorker(void* arg) {
    int ind = (int)arg;

    usleep(COEF * arr[ind].len);

    char buf[256]; 
    int err_code;
    if ((err_code = pthread_mutex_lock(&mut)) != 0) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Lock mut error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    sorted[ptr++] = arr[ind];

    if ((err_code = pthread_mutex_unlock(&mut)) != 0) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Unlock mut error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
}

int main() {
    char buf[256]; 
    int err_code;
    pthread_t threads[MAX_LINES];

    printf("Input up to 100 lines (Enter to stop):\n");
    
    int i = 0;
    for (i = 0; i < MAX_LINES; i++) {
        if (NULL == fgets(arr[i].str, BUFSIZ, stdin)) {
            break;
        }

        size_t len = strlen(arr[i].str);
        if ((len > 0) && (arr[i].str[len - 1] == '\n')) {
            len--;
            arr[i].str[len] = '\0';
        }
        arr[i].len = len;
        
        if (len == 0) {
            break;
        }
    }

    pthread_mutexattr_t attr;
    if ((err_code = pthread_mutexattr_init(&attr)) != 0) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Mutex attr init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((err_code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Mutex type set error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((err_code = pthread_mutex_init(&mut, &attr) != 0)) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Mutex mut init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    for (int j = 0; j < i; j++) {
        err_code = pthread_create(threads + j, NULL, sleepSortWorker, (void*)j);
        if (err_code != 0) {
            strerror_r(err_code, buf, sizeof(buf));
            fprintf(stderr, "Creating thread error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], NULL);
    }

    for (int j = 1; j < ptr; j++) {
        if (sorted[j].len < sorted[j - 1].len) {
            fprintf(stderr, "Sorting failed\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("\nSorted:\n");

    for (int j = 0; j < ptr; j++) {
        printf("%s\n", sorted[j].str);
    }

    if ((err_code = pthread_mutex_destroy(&mut)) != 0) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Mutex destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    
    if ((err_code = pthread_mutexattr_destroy(&attr)) != 0) {
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Mutex attr destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
