#ifndef PROXY_H
#define PROXY_H

//надо добавить сюда функции типа workLoop

#include <poll.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    char* buffer;
    ssize_t count;
    ssize_t len;
} Buffer;

typedef struct {
    Buffer* key;
    Buffer* val;
    time_t timeCreating;
    char status; 
    int inUse; 
} CacheEntry;


typedef struct {
    CacheEntry* buffers;
    char* state; //0 - empty, 1 - elem, 2 - tombstone
    unsigned int cap;
    unsigned int cnt;
} Cache;

typedef struct {
    int fd;
    ssize_t curIndex;
    Buffer* keyCache;
    Buffer* req;
} Request;

typedef struct {
    int fd;
    ssize_t curIndex;
    Buffer* key;
    Buffer* request;
} Loader;

typedef struct {
    struct pollfd* pfds;
    char* types; 
    Loader* loaders;
    Request* requests;
    int countLoaders;
    int lenLoaders;
    int countRequests;
    int lenRequests;
    int countPFDs; //количество "живых" записей
    int lenPFDs; //длина массива
    Cache cache;
    char endOfWork;
} ProxyState;


#define SOCKET 0 
#define ANSWER 1 
#define SERVER_ANSWER 2 
#define REQUEST 3
#define CONNECTING 4 
#define SENDING_REQ 5 

#define PAGE_CHECKING 0
#define PAGE_LOADING 1
#define PAGE_LOADED 2
#define PAGE_ERROR 3
#define PAGE_ERROR_LOADING 4
#define PAGE_SUCCESS_LOADING 5

#define HTTP_PORT 80

#define TIMEOUT_PURGE 300000

CacheEntry* getPage(Buffer* key);

int initCache();

int workLoop();

int putInCache(Buffer* key, Buffer* val, char status);

void purgeCache(time_t timeNow);

void removeFromCache(Buffer* keyCache);

#endif