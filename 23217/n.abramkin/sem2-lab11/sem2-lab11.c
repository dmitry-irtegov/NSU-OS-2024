#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t parent_mutex;
pthread_mutex_t child_mutex;
pthread_mutex_t sync_mutex;

void release_to_child() {
    pthread_mutex_unlock(&child_mutex);
    pthread_mutex_lock(&sync_mutex);
    pthread_mutex_unlock(&parent_mutex);
}

void release_to_parent() {
    pthread_mutex_lock(&child_mutex);
    pthread_mutex_unlock(&sync_mutex);
    pthread_mutex_lock(&parent_mutex);
}

void *thread_function(void *arg) {
    pthread_mutex_lock(&sync_mutex);
    for (int i = 0; i < 10; i++) {
        release_to_parent();
        printf("[Child thread]: Line %d\n", i + 1);
        release_to_child();
    }
    pthread_mutex_unlock(&sync_mutex);
    return NULL;
}

int main() {
    pthread_t thread;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&parent_mutex, &attr);
    pthread_mutex_init(&child_mutex, &attr);
    pthread_mutex_init(&sync_mutex, &attr);

    pthread_mutex_lock(&parent_mutex);

    pthread_create(&thread, NULL, thread_function, NULL);

    usleep(1000);

    for (int i = 0; i < 10; i++) {
        printf("[Parent thread]: Line %d\n", i + 1);
        release_to_child();
        release_to_parent();
    }

    pthread_join(thread, NULL);

    pthread_mutex_destroy(&parent_mutex);
    pthread_mutex_destroy(&child_mutex);
    pthread_mutex_destroy(&sync_mutex);
    
    pthread_mutexattr_destroy(&attr);

    return EXIT_SUCCESS;
}
