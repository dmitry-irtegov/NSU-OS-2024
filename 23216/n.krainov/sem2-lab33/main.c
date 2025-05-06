#include "proxy.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>

GlobalProxyState globalState;

void setFlag() {
    globalState.endOfWork = 1;
}

void* workerInit(void* ignored) {
    ignored = ignored + 10 - 10; //чтобы компилятор не жаловался

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
    
    WorkerState proxy;

    int initPFDs = 10;
    int initRequest = 10;
    int initLoaders = 10;

    proxy.countPFDs = 0;
    proxy.lenPFDs = initPFDs;
    proxy.countLoaders = 0;
    proxy.lenLoaders = initLoaders;
    proxy.countRequests = 0;
    proxy.lenRequests = initRequest;
    proxy.pfds = calloc(initPFDs, sizeof(struct pollfd));
    
    if (proxy.pfds == NULL) {
        return -1;
    }

    proxy.types = calloc(initPFDs, sizeof(char));

    if (proxy.types == NULL) {
        return -1;
    }

    proxy.loaders = calloc(initLoaders, sizeof(Loader));

    if (proxy.loaders == NULL) {
        return -1;
    }

    proxy.requests = calloc(initRequest, sizeof(Request));

    if (proxy.requests == NULL) {
        return -1;
    }

    for (int i = 0; i < initPFDs; i++) {
        proxy.pfds[i].fd = -1;
    }

    for (int i = 0; i < initRequest; i++) {
        proxy.requests[i].fd = -1;
    }

    for (int i = 0; i < initLoaders; i++) {
        proxy.loaders[i].fd = -1;
    }

    if (workerWorkLoop(&proxy)) {
        exit(EXIT_FAILURE);
    }

    return 0;
}

int initWorkers(int countWorkers) {
    globalState.workers = calloc(countWorkers, sizeof(pthread_t));
    if (globalState.workers == NULL) {
        return 1;
    }

    globalState.countWorkers = countWorkers;

    if (initQueue()) {
        return 1;
    }

    for (int i = 0; i < countWorkers; i++) {
        int err = 0;
        if ((err = pthread_create(&globalState.workers[i], NULL, workerInit, NULL))) {
            errno = err;
            return 1;
        }
    }

    return 0;
}

int initProxy(int port, int countWorkers) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        return 1;
    } 

    if (initCache()) {
        return 1;
    }

    globalState.pfds[0].fd = sockfd;
    globalState.pfds[0].events = POLLIN;

    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr));
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port);  
    
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        return 1;
    } 

    if (listen(sockfd, QUEUE_CAP) == -1) {
        return 1;
    }

    if (initWorkers(countWorkers)) {
        return 1;
    }

    if (signal(SIGINT, setFlag) == SIG_ERR) {
        return 1;
    }

    return 0;
}

int main(int argc, char** argv) {
    int port, countWorkers;
    if (argc < 3 || (port = atoi(argv[1])) <= 1024 || (countWorkers = atoi(argv[2])) <= 0) {
        fprintf(stderr, "usage: %s port countofWorkers\n", argv[0]);
        exit(1);
    }

    if (initProxy(port, countWorkers)) { 
        perror("initProxy failed");
        exit(1);
    }

    if (managerWorkLoop()) { 
        perror("error in workloop");
        exit(2);
    }

    exit(EXIT_SUCCESS);
}