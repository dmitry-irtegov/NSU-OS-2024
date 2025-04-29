#include "proxy.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern ProxyState proxy;

int initCache() {
    proxy.cache.buffers = calloc(10, sizeof(CacheEntry));
    if (proxy.cache.buffers == NULL) {
        return 1;
    }

    proxy.cache.state = calloc(10, sizeof(char));
    if (proxy.cache.state == NULL) {
        return 1;
    }

    proxy.cache.cap = 10;
    proxy.cache.cnt = 0;
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
    CacheEntry* newBuffers = calloc(proxy.cache.cap * 2, sizeof(CacheEntry));
    if (newBuffers == NULL) {
        return 1;
    }
    char* newStates = calloc(proxy.cache.cap * 2, sizeof(char));
    if (newStates == NULL) {
        return 1;
    }
    int newCount = 0;

    for (unsigned int j = 0; j < proxy.cache.cap; j++) {
        if (proxy.cache.state[j] == 1) {
            int index = getHash(proxy.cache.buffers[j].key->buffer, proxy.cache.buffers[j].key->count) % proxy.cache.cap;

            for (int i = index; ; i = (i + 1) % (proxy.cache.cap * 2)) {
                if (newStates[i] == 0) {
                    newBuffers[i].key = proxy.cache.buffers[j].key;
                    newBuffers[i].val = proxy.cache.buffers[j].val;
                    newBuffers[i].status = proxy.cache.state[j];
                    newStates[i] = 1;
                    newCount++;
                    break;
                }
            }
        }
    }


    free(proxy.cache.buffers);
    free(proxy.cache.state);
    proxy.cache.buffers = newBuffers;
    proxy.cache.state = newStates;
    proxy.cache.cap *= 2;
    proxy.cache.cnt = newCount;

    return 0;
}

CacheEntry* getPage(Buffer* key) {
    unsigned int index = getHash(key->buffer, key->count) % proxy.cache.cap;
    
    for (unsigned int i = index; ; i = (i + 1) % proxy.cache.cap) {
        if (proxy.cache.state[i] == 0) {
            break;
        }

        if (proxy.cache.state[i] == 2) {
            continue;
        }

        if (key->count != proxy.cache.buffers[i].key->count) continue;

        if (strncmp(proxy.cache.buffers[i].key->buffer, key->buffer, key->count) == 0) { 
            return &proxy.cache.buffers[i];
        }
    }

    return NULL;
}

int putInCache(Buffer* key, Buffer* val, char status) {
    unsigned int index = getHash(key->buffer, key->count) % proxy.cache.cap;

    if (proxy.cache.cnt >= 0.75 * proxy.cache.cap) {
        if (resizeCache()) {
            return 1;
        }
    } 

    for (unsigned int i = index; ; i = (i + 1) % proxy.cache.cap) {
        if (proxy.cache.state[i] == 0) {
            proxy.cache.buffers[i].key = key;
            proxy.cache.buffers[i].val = val;
            proxy.cache.buffers[i].status = status;
            proxy.cache.buffers[i].inUse = 2;
            proxy.cache.state[i] = 1;
            return 0;
        }
    }

    return 1;
}

void removeFromCache(Buffer* key) {
    unsigned int index = getHash(key->buffer, key->count) % proxy.cache.cap;
    
    for (unsigned int i = index; ; i = (i + 1) % proxy.cache.cap) {
        if (proxy.cache.state[i] == 0) {
            break;
        }

        if (proxy.cache.state[i] == 2) {
            continue;
        }

        if (key->count != proxy.cache.buffers[i].key->count) continue;

        if (strncmp(proxy.cache.buffers[i].key->buffer, key->buffer, key->count) == 0) { 
            proxy.cache.state[i] = 2;
            freeBuffer(proxy.cache.buffers[i].val);
            freeBuffer(proxy.cache.buffers[i].key);
        }
    }
}

void purgeCache(time_t timeNow) {
    for (unsigned int i = 0; i < proxy.cache.cap; i++) {
        if (proxy.cache.state[i] == 1 && proxy.cache.buffers[i].inUse == 0 && difftime(timeNow, proxy.cache.buffers[i].timeCreating) >= 300) {
            proxy.cache.state[i] = 2;
            freeBuffer(proxy.cache.buffers[i].key);
            freeBuffer(proxy.cache.buffers[i].val);
        }
    }
}