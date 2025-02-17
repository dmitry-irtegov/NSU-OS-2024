#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int turn = 0;

void print_all_lines(char* string, int thread_id) {
    for (int i = 0; i < 10; i++) {
        int result = pthread_mutex_lock(&mutex);
        if (result != 0)
            handle_error_en(result, "pthread_mutex_lock");

        while (turn != thread_id) {
            result = pthread_cond_wait(&cond, &mutex);
            if (result != 0)
                handle_error_en(result, "pthread_cond_wait");
        }
        printf("%s: number %d\n", string, i);
        turn = 1 - turn;

        result = pthread_cond_signal(&cond);
        if (result != 0)
            handle_error_en(result, "pthread_cond_signal");

        result = pthread_mutex_unlock(&mutex);
        if (result != 0)
            handle_error_en(result, "pthread_mutex_unlock");
    }
}

void *print_lines(void* arg) {
    print_all_lines("Thread", 1);
    pthread_exit(0);
}

int main() {
    pthread_t thread;
    pthread_attr_t attr;
    int result;

    result = pthread_attr_init(&attr);
    if (result != 0)
        handle_error_en(result, "pthread_attr_init");

    result = pthread_create(&thread, &attr, &print_lines, NULL);
    if (result != 0)
        handle_error_en(result, "pthread_create");

    result = pthread_attr_destroy(&attr);
    if (result != 0)
        handle_error_en(result, "pthread_attr_destroy");

    print_all_lines("Parent thread", 0);

    result = pthread_join(thread, NULL);
    if (result != 0)
        handle_error_en(result, "pthread_join");

    result = pthread_mutex_destroy(&mutex);
    if (result != 0)
        handle_error_en(result, "pthread_mutex_destroy");

    result = pthread_cond_destroy(&cond);
    if (result != 0)
        handle_error_en(result, "pthread_cond_destroy");

    pthread_exit(0);
}
