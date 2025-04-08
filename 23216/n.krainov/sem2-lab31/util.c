#include "util.h"
#include "proxy.h"
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

extern ProxyState proxy;

Buffer* initBuffer(int cap, char* source) {
    Buffer* ret = calloc(1, sizeof(Buffer));
    if (ret == NULL) {
        return NULL;
    }

    cap++;

    ret->buffer = calloc(cap, sizeof(char));
    if (ret->buffer == NULL) {
        free(ret);
        return NULL;
    }

    if (source) ret->count = cap - 1;
    else ret->count = 0;

    ret->len = cap - 1;

    
    if (source) strncpy(ret->buffer, source, ret->len);

    return ret;
}

void freeBuffer(Buffer* buf) {
    if (buf != NULL) {
        free(buf->buffer);
        free(buf);
    }
}

int resizeBuffer(Buffer* buf) {
    if (buf->count == buf->len) {
        buf->buffer = realloc(buf->buffer, sizeof(char) * (buf->len * 2 + 1));
        if (buf->buffer == NULL) {
            return 1;
        }

        buf->buffer[buf->len * 2] = '\0';
        buf->len = buf->len * 2 + 1;
    }

    return 0;
}

int addToPFDs(int newConn, short event, char type) {
    int startIndex = 1;
    if (proxy.countPFDs == proxy.lenPFDs) {
        proxy.pfds = realloc(proxy.pfds, proxy.lenPFDs * 2);
        if (proxy.pfds == NULL) { 
            return 1;
        }

        proxy.types = realloc(proxy.types, proxy.lenPFDs * 2);
        if (proxy.types == NULL) {
            return 1;
        }

        proxy.lenPFDs = proxy.lenPFDs * 2;
        startIndex = proxy.countPFDs;

        for (int i = startIndex; i < proxy.lenPFDs; i++) {
            proxy.pfds[i].fd = -1;
        }
    }

    for (int i = startIndex; i < proxy.lenPFDs; i++) {  
        if (proxy.pfds[i].fd == -1) {
            proxy.pfds[i].fd = newConn;
            proxy.pfds[i].events = event;
            proxy.types[i] = type;
            proxy.countPFDs++;
            
            return 0;
        }
    }

    return 1;
}

int addToRequests(int newConn) {
    int startIndex = 0;
    if (proxy.countRequests == proxy.lenRequests) {
        proxy.requests = realloc(proxy.requests, proxy.lenRequests * 2);   
        if (proxy.requests == NULL) {
            return 1;
        }

        startIndex = proxy.countRequests;
    }

    for (int i = startIndex; i < proxy.lenRequests; i++) { //
        if (proxy.requests[i].fd == -1) {
            proxy.requests[i].fd = newConn;
            
            proxy.requests[i].req = initBuffer(256, NULL);

            if (proxy.requests[i].req == NULL) {
                return 1;
            }

            return 0;
        }
    }

    return 1;
}

Request* findRequest(int indexPFD) {
    for (int i = 0; i < proxy.lenRequests; i++) { 
        if (proxy.requests[i].fd == proxy.pfds[indexPFD].fd) {
            return &proxy.requests[i];
        }
    }

    //not reachable
    return NULL;
}

Loader* findLoader(int indexPFD) {
    for (int i = 0; i < proxy.lenLoaders; i++) { 
        if (proxy.loaders[i].fd == proxy.pfds[indexPFD].fd) {
            return &proxy.loaders[i];
        }
    }

    //not reachable
    return NULL;
}

int checkEndOfReq(Buffer* req) {
    return req->buffer[req->count-1] == '\n' && req->buffer[req->count-2] == '\r' 
        && req->buffer[req->count-3] == '\n' && req->buffer[req->count-4] == '\r';    
}