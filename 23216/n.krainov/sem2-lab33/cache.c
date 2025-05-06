#include "proxy.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

extern GlobalProxyState globalState;

int initCache() {
    int err = 0;
    if ((err = pthread_rwlock_init(&globalState.cache.lock, NULL))) {
        errno = err;
        return 1;
    }

    globalState.cache.buffers = calloc(10, sizeof(CacheEntry));
    if (globalState.cache.buffers == NULL) {
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        if ((err = pthread_mutex_init(&globalState.cache.buffers[i].mutex, NULL))) {
            return 1;
        }
    }

    globalState.cache.state = calloc(10, sizeof(char));
    if (globalState.cache.state == NULL) {
        return 1;
    }

    globalState.cache.cap = 10;
    globalState.cache.cnt = 0;
    return 0;
}

unsigned int getHash(char* key, int len) {
    int k = 265;
    unsigned long long h = 0;
    for (int i = 0; i < len; i++) {
        h = (h * k + key[i]) % 1000000007; 
    }

    return h; 
}


int resizeCache() {
    CacheEntry* newBuffers = calloc(globalState.cache.cap * 2, sizeof(CacheEntry));
    if (newBuffers == NULL) {
        return 1;
    }
    char* newStates = calloc(globalState.cache.cap * 2, sizeof(char));
    if (newStates == NULL) {
        return 1;
    }
    int newCount = 0;

    for (unsigned int j = 0; j < globalState.cache.cap; j++) {
        if (globalState.cache.state[j] == 1) {
            int index = getHash(globalState.cache.buffers[j].key->buffer, globalState.cache.buffers[j].key->count) % globalState.cache.cap;

            for (int i = index; ; i = (i + 1) % (globalState.cache.cap * 2)) {
                if (newStates[i] == 0) {
                    newBuffers[i].key = globalState.cache.buffers[j].key;
                    newBuffers[i].val = globalState.cache.buffers[j].val;
                    newBuffers[i].status = globalState.cache.state[j];
                    newStates[i] = 1;
                    newCount++;
                    break;
                }
            }
        }
    }


    free(globalState.cache.buffers);
    free(globalState.cache.state);
    globalState.cache.buffers = newBuffers;
    globalState.cache.state = newStates;
    globalState.cache.cap *= 2;
    globalState.cache.cnt = newCount;

    return 0;
}

CacheEntry* getPage(Buffer* key) {
    pthread_rwlock_rdlock(&globalState.cache.lock);
    unsigned int index = getHash(key->buffer, key->count) % globalState.cache.cap;
    
    for (unsigned int i = index; ; i = (i + 1) % globalState.cache.cap) {
        if (globalState.cache.state[i] == 0) {
            break;
        }

        if (globalState.cache.state[i] == 2) {
            continue;
        }

        if (key->count != globalState.cache.buffers[i].key->count) continue;

        if (strncmp(globalState.cache.buffers[i].key->buffer, key->buffer, key->count) == 0) { 
            pthread_rwlock_unlock(&globalState.cache.lock);
            return &globalState.cache.buffers[i];
        }
    }

    pthread_rwlock_unlock(&globalState.cache.lock);
    return NULL;
}

int putInCache(Buffer* key, Buffer* val, char status) {
    pthread_rwlock_wrlock(&globalState.cache.lock);
    unsigned int index = getHash(key->buffer, key->count) % globalState.cache.cap;

    if (globalState.cache.cnt >= 0.75 * globalState.cache.cap) {
        if (resizeCache()) {
            pthread_rwlock_unlock(&globalState.cache.lock);
            return 1;
        }
    } 

    for (unsigned int i = index; ; i = (i + 1) % globalState.cache.cap) {
        if (globalState.cache.state[i] == 0) {
            globalState.cache.buffers[i].key = key;
            globalState.cache.buffers[i].val = val;
            globalState.cache.buffers[i].status = status;
            globalState.cache.buffers[i].inUse = 2;
            globalState.cache.state[i] = 1;
            pthread_rwlock_unlock(&globalState.cache.lock);
            return 0;
        }
    }

    pthread_rwlock_unlock(&globalState.cache.lock);
    return 1;
}

void removeFromCache(Buffer* key) {
    pthread_rwlock_wrlock(&globalState.cache.lock);
    unsigned int index = getHash(key->buffer, key->count) % globalState.cache.cap;
    
    for (unsigned int i = index; ; i = (i + 1) % globalState.cache.cap) {
        if (globalState.cache.state[i] == 0) {
            break;
        }

        if (globalState.cache.state[i] == 2) {
            continue;
        }

        if (key->count != globalState.cache.buffers[i].key->count) continue;

        if (strncmp(globalState.cache.buffers[i].key->buffer, key->buffer, key->count) == 0) { 
            globalState.cache.state[i] = 2;
            freeBuffer(globalState.cache.buffers[i].val);
            freeBuffer(globalState.cache.buffers[i].key);
            pthread_rwlock_unlock(&globalState.cache.lock);

            return;
        }
    }

    pthread_rwlock_unlock(&globalState.cache.lock);
}

void purgeCache(time_t timeNow) {
    pthread_rwlock_wrlock(&globalState.cache.lock);
    for (unsigned int i = 0; i < globalState.cache.cap; i++) {
        if (globalState.cache.state[i] == 1 && globalState.cache.buffers[i].inUse == 0 && difftime(timeNow, globalState.cache.buffers[i].timeCreating) >= 300) {
            globalState.cache.state[i] = 2;
            freeBuffer(globalState.cache.buffers[i].key);
            freeBuffer(globalState.cache.buffers[i].val);
        }
    }
    pthread_rwlock_unlock(&globalState.cache.lock);
}