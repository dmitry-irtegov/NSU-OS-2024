#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define handle_error_en(en, msg) \
    do { \
        errno = en; \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0)

pthread_mutex_t mutex;
int flag = 0;
int global_count = 10;

void print_all_lines(char *string, int i) {
    int s;
    if ((s = pthread_mutex_lock(&mutex)) != 0) {
        handle_error_en(s, "pthread_mutex_lock");
    }

    printf("%s: number %d\n", string, i); 

    if ((s = pthread_mutex_unlock(&mutex)) != 0) {
        handle_error_en(s, "pthread_mutex_unlock");
    }
}

void *print_lines(void *arg) {
    int count = 0;

    while (count < global_count) {
        if (flag == 1) {
            print_all_lines("Thread", count);
            count++;
            flag = 0;
        }
    }

    pthread_exit(EXIT_SUCCESS);
}

int main() {
    pthread_t thread;
    int s, count = 0;

    pthread_mutexattr_t attr;
    if ((s = pthread_mutexattr_init(&attr)) != 0) {
        handle_error_en(s, "pthread_mutexattr_init");
    }

    if ((s = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        handle_error_en(s, "pthread_mutexattr_settype");
        pthread_mutexattr_destroy(&attr);
    }

    if ((s = pthread_mutex_init(&mutex, &attr)) != 0) {
        handle_error_en(s, "pthread_mutex_init");
        pthread_mutexattr_destroy(&attr);
    }

    if ((s = pthread_create(&thread, NULL, print_lines, NULL)) != 0) {
        handle_error_en(s, "pthread_create");
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy(&attr);
    }

    while (count < global_count) {
        if (flag == 0) {
            print_all_lines("Parent thread", count);
            count++;
            flag = 1;
        }
    }

    if ((s = pthread_join(thread, NULL)) != 0) {
        handle_error_en(s, "pthread_join");
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy(&attr);
    }

    if ((s = pthread_mutex_destroy(&mutex)) != 0) {
        handle_error_en(s, "pthread_mutex_destroy");
        pthread_mutexattr_destroy(&attr);
    }

    if ((s = pthread_mutexattr_destroy(&attr)) != 0) {
        handle_error_en(s, "pthread_mutexattr_destroy");
    }

    exit(EXIT_SUCCESS);
}
