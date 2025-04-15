#include "util.h"
#include "proxy.h"
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

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
        buf->len = buf->len * 2;
    }

    return 0;
}

int addToPFDs(int newConn, short event, char type) {
    int startIndex = 1;
    if (proxy.countPFDs == proxy.lenPFDs) {
        struct pollfd* pfds = calloc(proxy.lenPFDs * 2, sizeof(struct pollfd));
        char* types = calloc(proxy.lenPFDs * 2, sizeof(char));

        if (pfds == NULL || types == NULL) {
            free(pfds);
            free(types);
            return 1;
        }

        memcpy(pfds, proxy.pfds, sizeof(struct pollfd) * proxy.lenPFDs);
        memcpy(types, proxy.types, sizeof(char) * proxy.lenPFDs);
        
        free(proxy.pfds);
        free(proxy.types);

        proxy.pfds = pfds;
        proxy.types = types;

        proxy.lenPFDs = proxy.lenPFDs * 2;
        startIndex = proxy.countPFDs;

        for (int i = startIndex; i < proxy.lenPFDs; i++) {
            proxy.pfds[i].fd = -1;
            proxy.types[i] = 0;
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

void removeFromPFDs(int newConn) {
    for (int i = 1; i < proxy.lenPFDs; i++) {  
        if (proxy.pfds[i].fd == newConn) {
            proxy.pfds[i].fd = -1;
            proxy.pfds[i].events = 0;
            proxy.types[i] = 0;
            proxy.countPFDs--;
            return;
        }
    }
}

int addToRequests(int newConn) {
    int startIndex = 0;
    if (proxy.countRequests == proxy.lenRequests) {
        Request* res = realloc(proxy.requests, sizeof(Request) * proxy.lenRequests * 2);   
        if (res == NULL) {
            return 1;
        }
        proxy.requests = res;

        startIndex = proxy.countRequests;
        proxy.lenRequests = proxy.lenRequests * 2;

        for (int i = startIndex; i < proxy.lenRequests; i++) {
            proxy.requests[i].fd = -1;
        }
    }

    for (int i = startIndex; i < proxy.lenRequests; i++) { //
        if (proxy.requests[i].fd == -1) {
            proxy.requests[i].req = initBuffer(128, NULL);

            if (proxy.requests[i].req == NULL) {
                return 1;
            }

            proxy.countRequests++;
            proxy.requests[i].fd = newConn;

            return 0;
        }
    }

    return 1;
}

void removeFromRequests(int newConn) {
    for (int i = 0; i < proxy.lenRequests; i++) {  
        if (proxy.requests[i].fd == newConn) {
            proxy.requests[i].fd = -1;
            freeBuffer(proxy.requests[i].keyCache);
            freeBuffer(proxy.requests[i].req);
            proxy.countRequests--;
            return;
        }
    }
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
    return strstr(req->buffer, "\r\n\r\n") != NULL;    
}

int analyzeRequest(Request* req) {
    char* check1 = strstr(req->req->buffer, "HEAD");
    char* check2 = strstr(req->req->buffer, "GET");
    char* check3 = strstr(req->req->buffer, "HTTP/1.0");

    if ((check1 == NULL) == (check2 == NULL) || !check3 || ((check1 == NULL) ? check2 : check1) > check3) { 
        return 1;
    }

    char* http = strstr(req->req->buffer, "http://");
    char* HTTP = strstr(req->req->buffer, "HTTP://");

    char* url = (http == NULL ? HTTP : http);
    if ((http == NULL) == (HTTP == NULL) && url < check3 &&
        url > ((check1 == NULL) ? check2 : check1)) {

        return 1;
    }

    url += strlen("HTTP://");

    char* ptr = url;
    int cap = 0;

    while (!isspace(*ptr)) {
        cap++;
        ptr++;
    }
    
    if (cap == 0) {
        return 1;
    }

    Buffer* key = initBuffer(cap, url);
    if (key == NULL) {
        return -1;
    }
    
    req->keyCache = key;

    return 0;
}