#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static pthread_mutex_t parent_mutex;
static pthread_mutex_t child_mutex;

void *print_lines(void *arg) {
    int result;
    for (int i = 0; i < 10; i++) {
        result = pthread_mutex_lock(&child_mutex);
        if (result != 0)
            handle_error_en(result, "pthread_mutex_lock");
        printf("Thread: number %d\n", i);
        result = pthread_mutex_unlock(&parent_mutex);
        if (result != 0)
            handle_error_en(result, "pthread_mutex_unlock");
    }
    pthread_exit(0);
}

int main() {
    pthread_t thread;
    int result;

    pthread_mutexattr_t mutex_attr;
    result = pthread_mutexattr_init(&mutex_attr);
    if (result != 0)
        handle_error_en(result, "pthread_mutexattr_init");
    result = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_NORMAL);
    if (result != 0)
        handle_error_en(result, "pthread_mutexattr_settype");

    result = pthread_mutex_init(&parent_mutex, &mutex_attr);
    if (result != 0)
        handle_error_en(result, "pthread_mutex_ini");
    result = pthread_mutex_init(&child_mutex, &mutex_attr);
    if (result != 0)
        handle_error_en(result, "pthread_mutex_init");

    pthread_mutexattr_destroy(&mutex_attr);

    result = pthread_mutex_lock(&parent_mutex);
    if (result != 0)
        handle_error_en(result, "pthread_mutex_lock");

    result = pthread_mutex_lock(&child_mutex);
    if (result != 0)
        handle_error_en(result, "pthread_mutex_lock");

    result = pthread_create(&thread, NULL, print_lines, NULL);
    if (result != 0)
        handle_error_en(result, "pthread_create");

    for (int i = 0; i < 10; i++) {
        printf("Parent thread: number %d\n", i);
        result = pthread_mutex_unlock(&child_mutex);
        if (result != 0)
            handle_error_en(result, "pthread_mutex_unlock");
        if (i < 9) {
            result = pthread_mutex_lock(&parent_mutex);
            if (result != 0)
                handle_error_en(result, "pthread_mutex_lock");
        }
    }

    result = pthread_join(thread, NULL);
    if (result != 0)
        handle_error_en(result, "pthread_join");

    pthread_mutex_destroy(&parent_mutex);
    pthread_mutex_destroy(&child_mutex);

    pthread_exit(0);
}