#include "proxy.h"
#include <stdlib.h>
#include <string.h>

extern ProxyState proxy;

int initCache() {
    proxy.cache.buffers = calloc(10, sizeof(Buffer));
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

    return h % proxy.cache.cap;
}

CacheEntry* getPage(Buffer* key) {
    unsigned int index = getHash(key->buffer, key->count);
    
    //resize

    for (unsigned int i = index; ; i = (i + 1) % proxy.cache.cap) {
        if (proxy.cache.state[i] == 0) {
            break;
        }

        if (proxy.cache.state[i] == 2) {
            continue;
        }

        if (strncmp(proxy.cache.buffers[i].key->buffer, key->buffer, key->count) == 0) {
            return &proxy.cache.buffers[i];
        }
    }

    return NULL;
}

void putInCache(Buffer* key, Buffer* val) {
    int index = getHash(key->buffer, key->count);
    
    //resize

    for (int i = index; ; i = (i + 1) % proxy.cache.cap) {
        if (proxy.cache.state[i] == 0) {
            proxy.cache.buffers[i].key = key;
            proxy.cache.buffers[i].val = val;
            proxy.cache.buffers[i].status = 0;
            proxy.cache.state[i] = 1;
            return;
        }
    }

}