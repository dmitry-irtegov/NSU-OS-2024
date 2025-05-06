#include "proxy.h"
#include <stdlib.h>
#include <errno.h>

extern GlobalProxyState globalState;

int initQueue() {
    if (sem_init(&globalState.queue.semaphoreGet, 0, 0)) {
        return -1;
    }
    if (sem_init(&globalState.queue.semaphorePut, 0, QUEUE_CAP)) {
        return -1;
    }
    int err;
    if ((err = pthread_mutex_init(&globalState.queue.mutex, NULL))) {
        errno = err;
        return -1;
    }
    globalState.queue.start = 0;
    globalState.queue.end = 0;

    globalState.queue.queue = calloc(QUEUE_CAP, sizeof(int));
    if (globalState.queue.queue == NULL) {
        return -1;
    }

    return 0;
}

int putReq(int fd) {
    int err = 0;
    if (globalState.endOfWork == 0) {
        if (sem_wait(&globalState.queue.semaphorePut)) {
            return -1;
        }
    }

    if ((err = pthread_mutex_lock(&globalState.queue.mutex))) {
        errno = err;
        return -1;
    }

    if (fd == -1) {
        for (int i = 0; i < globalState.countWorkers; i++) {
            sem_post(&globalState.queue.semaphoreGet);
        }
        globalState.queue.endOfWork = 1;
    } else {
        globalState.queue.queue[globalState.queue.end] = fd;
        globalState.queue.end = (globalState.queue.end + 1) % QUEUE_CAP;
    }

    if ((err = pthread_mutex_unlock(&globalState.queue.mutex))) {
        errno = err;
        return -1;
    }

    if (sem_post(&globalState.queue.semaphoreGet)) {
        return -1;
    }

    return 0;

}

int getReq() {
    int err, res;

    if (sem_wait(&globalState.queue.semaphoreGet)) {
        return -1;
    }

    if ((err = pthread_mutex_lock(&globalState.queue.mutex))) {
        errno = err;
        return -1;
    }

    if (globalState.queue.endOfWork) {
        res = -2;
    } else {
        res = globalState.queue.queue[globalState.queue.start];
        globalState.queue.start = (globalState.queue.start + 1) % QUEUE_CAP;
    }

    if ((err = pthread_mutex_unlock(&globalState.queue.mutex))) {
        errno = err;
        return -1;
    }

    if (sem_post(&globalState.queue.semaphorePut)) {
        return -1;
    }
    
    return res;
}

int tryGetReq() {
    int err, res;

    if (sem_trywait(&globalState.queue.semaphoreGet)) {
        if (errno == EAGAIN) {
            return -2;
        }
        return -1;
    }

    if ((err = pthread_mutex_lock(&globalState.queue.mutex))) {
        errno = err;
        return -1;
    }

    res = globalState.queue.queue[globalState.queue.start];
    globalState.queue.start = (globalState.queue.start + 1) % QUEUE_CAP;
    

    if ((err = pthread_mutex_unlock(&globalState.queue.mutex))) {
        errno = err;
        return -1;
    }

    if (sem_post(&globalState.queue.semaphorePut)) {
        return -1;
    }
    
    return res;
}