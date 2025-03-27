#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

pthread_mutex_t parent;
pthread_mutex_t child;
pthread_mutex_t common;

void *child_thread() {
    assert(pthread_mutex_lock(&child) == 0);
    for (int i = 0; i < 10; i++) {
        assert(pthread_mutex_lock(&parent) == 0);
        printf("Child: %d\n", i);
        assert(pthread_mutex_unlock(&child) == 0);
        assert(pthread_mutex_lock(&common) == 0);
        assert(pthread_mutex_unlock(&parent) == 0);
        assert(pthread_mutex_lock(&child) == 0);
        assert(pthread_mutex_unlock(&common) == 0);
    }
    assert(pthread_mutex_unlock(&child) == 0);
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
        perror("init mutex atribute failed");
        exit(-1);
    }
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        perror("set mutex atribute failed");
        exit(-1);
    }

    if(pthread_mutex_init(&parent, &attr) != 0) {
        perror("init mutex failed");
        exit(-1);
    }

    if(pthread_mutex_init(&child, &attr) != 0) {
        perror("init mutex failed");
        exit(-1);
    }

    if(pthread_mutex_init(&common, &attr) != 0) {
        perror("init mutex failed");
        exit(-1);
    }

    assert(pthread_mutex_lock(&parent) == 0);

    if (pthread_create(&thread, NULL, child_thread, NULL) != 0) {
        perror("thread create failed");
        exit(-1);
    }

    for (int i = 0; i < 10; i++) {
        printf("Parent: %d\n", i);
        assert(pthread_mutex_lock(&common) == 0);
        assert(pthread_mutex_unlock(&parent) == 0);
        assert(pthread_mutex_lock(&child) == 0);
        assert(pthread_mutex_unlock(&common) == 0);
        assert(pthread_mutex_lock(&parent) == 0);
        assert(pthread_mutex_unlock(&child) == 0);
    }
    assert(pthread_mutex_unlock(&parent) == 0);

    if (pthread_join(thread, NULL) != 0) {
        perror("join failed");
        exit(-1);
    }

    if (pthread_mutex_destroy(&parent) != 0) {
        perror("destroy parent mutex failed");
        exit(-1);
    }

    if (pthread_mutex_destroy(&child) != 0) {
        perror("destroy child mutex failed");
        exit(-1);
    }

    if (pthread_mutex_destroy(&common) != 0) {
        perror("destroy common mutex failed");
        exit(-1);
    }

    pthread_mutexattr_destroy(&attr);
    exit(0);
}