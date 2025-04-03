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
    char status; // 0 - качается, 1 - скачано, 2 - закачивание провалилось (на будущее) (нужно еще состояний добавить)
    int ttl; //на будущее (надо подумать, когда чистить кэш. Возможно, проще раз в какое-то время устраивать глобальную чистку (типа сборщика мусора))
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
} ProxyState;


#define SOCKET 0 
#define ANSWER 1 //output
#define SERVER_ANSWER 2 //input
#define REQUEST 3 //input
#define CONNECTING 4 //output
#define SENDING_REQ 5 //output

#define HTTP_PORT 80

CacheEntry* getPage(Buffer* key);

#endif