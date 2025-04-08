#include "proxy.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#define MAX 10

ProxyState proxy;

void setFlag() {
    proxy.endOfWork = 1;
}

int initProxy(int port) {
    int initPFDs = 10;
    int initRequest = 10;
    int initLoaders = 10;

    proxy.endOfWork = 0;
    proxy.countPFDs = 1;
    proxy.lenPFDs = initPFDs;
    proxy.countLoaders = 0;
    proxy.lenLoaders = initLoaders;
    proxy.countRequests = 0;
    proxy.lenRequests = initRequest;
    proxy.pfds = calloc(initPFDs, sizeof(struct pollfd));
    
    if (proxy.pfds == NULL) {
        return 1;
    }

    proxy.types = calloc(initPFDs, sizeof(char));

    if (proxy.types == NULL) {
        return 1;
    }

    proxy.loaders = calloc(initLoaders, sizeof(Loader));

    if (proxy.loaders == NULL) {
        return 1;
    }

    proxy.requests = calloc(initRequest, sizeof(Request));

    if (proxy.requests == NULL) {
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        return 1;
    } 

    if (initCache()) {
        return 1;
    }

    proxy.pfds[0].fd = sockfd;
    proxy.pfds[0].events = POLLIN;
    
    for (int i = 1; i < initPFDs; i++) {
        proxy.pfds[i].fd = -1;
    }

    for (int i = 0; i < initRequest; i++) {
        proxy.requests[i].fd = -1;
    }

    for (int i = 0; i < initLoaders; i++) {
        proxy.loaders[i].fd = -1;
    }

    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr));
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port);  
    
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        return 1;
    } 

    if (listen(sockfd, 10) == -1) {
        return 1;
    }

    if (sigset(SIGINT, setFlag) == SIG_ERR) {
        return 1;
    }

    return 0;
}

//ПРОКСИ ЕЩЕ НЕ ЗАВЕРШЕН
//если ты пришел смотреть как делать его, то лучше подожди, когда будет готово
//Или почитай комментарии, что доделывать надо
int main(int argc, char** argv) {
    int port;
    if (argc < 2 || (port = atoi(argv[1])) <= 1024) {
        fprintf(stderr, "incorrect port\n");
        exit(1);
    }

    if (initProxy(port)) { 
        perror("initProxy failed");
        exit(1);
    }

    if (workLoop()) { //надо бы и успешное окончание работы предусмотреть
        perror("error in workloop");
        exit(2);
    }

    exit(EXIT_SUCCESS);
}