#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netdb.h>

#define PORT "80"
#define HOST "example.com"
#define REQUEST "GET / HTTP/1.1\r\nHost: " HOST "\r\nConnection: close\r\n\r\n"
#define BUFFER_SIZE 256

typedef struct grar_s {
    pthread_mutex_t mutex;
    char* str;
    char curchar;
    int curlen;
    int maxlen;
    int curpos;
} grar_t;

void grar_init(grar_t* arr)
{
    pthread_mutex_init(&(arr->mutex), NULL);
    arr->curlen = 0;
    arr->maxlen = BUFFER_SIZE;
    arr->curpos = 0;
    arr->curchar = 0;
    arr->str = (char*)calloc(BUFFER_SIZE, sizeof(char));
}

void grar_addbuf(grar_t* arr, char* buf)
{
    pthread_mutex_lock(&(arr->mutex));
    if(arr->curlen + BUFFER_SIZE >= arr->maxlen) {
        arr->maxlen *= 2;
        arr->str = (char*)realloc(arr->str, arr->maxlen);
    }
    memcpy(arr->str + arr->curlen, buf, BUFFER_SIZE);
    pthread_mutex_unlock(&(arr->mutex));
}

void grar_getstr(grar_t* arr, char** start)
{
    pthread_mutex_lock(&(arr->mutex));
    arr->str[arr->curpos] = arr->curchar;
    for(int i = arr->curpos; i < arr->curlen - 1; i++) {
        if(arr->str[i] == '\n') {
            arr->curchar = arr->str[i + 1];
            arr->str[i + 1] = 0;
            *start = arr->curpos;
            arr->curpos = arr->str + i + 1;
            pthread_mutex_unlock(&(arr->mutex));
            return;
        }
    }
    arr->curchar = arr->str[arr->curlen - 1];
    arr->str[arr->curlen - 1] = 0;
    *start = arr->curpos;
    arr->curpos = arr->curlen;
    pthread_mutex_unlock(&(arr->mutex));
}



void* thread_printer(void* arr)
{
    while(1) {
        char* sbegin = NULL;
        grar_getstr((grar_t*)arr, &sbegin);
        printf("%s", sbegin);
        sleep(1);
    }
}

int main() {
    int status, socket_fd;
    struct addrinfo adrinf, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    char buffer[BUFFER_SIZE];

    memset(&adrinf, 0, sizeof adrinf);
    adrinf.ai_family = AF_UNSPEC;
    adrinf.ai_socktype = SOCK_STREAM;

    if (status = getaddrinfo(HOST, PORT, &adrinf, &res)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    for(p = res; p != NULL; p = p->ai_next) {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1) {
            perror("socket");
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd);
            perror("connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        freeaddrinfo(res);
        return 2;
    }

    freeaddrinfo(res);

    ssize_t bytes_sent = send(socket_fd, REQUEST, strlen(REQUEST), 0);
    if (bytes_sent == -1) {
        perror("send");
        close(socket_fd);
        return 3;
    }

    grar_t arr;
    grar_init(&arr);

    pthread_t thread;
    pthread_create(&thread, NULL, thread_printer, (void*)&arr);

    ssize_t bytes_received;
    while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        grar_addbuf(&arr, buffer);
    }

    if (bytes_received == -1) {
        fprintf(stderr, "unable to receive bytes");
    }

    close(socket_fd);
    return 0;
}

