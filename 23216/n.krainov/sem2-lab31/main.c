#include "proxy.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

#define MAX 10

ProxyState proxy;

int initProxy() {
    proxy.countPFDs = 0;
    proxy.lenPFDs = 10;
    proxy.countLoaders = 0;
    proxy.lenLoaders = 10;
    proxy.countRequests = 0;
    proxy.lenRequests = 10;
    proxy.pfds = calloc(10, sizeof(struct pollfd));
    
    if (proxy.pfds == NULL) {
        return 1;
    }

    proxy.types = calloc(10, sizeof(char));

    if (proxy.types == NULL) {
        return 1;
    }

    proxy.loaders = calloc(10, sizeof(Loader));

    if (proxy.loaders == NULL) {
        return 1;
    }

    proxy.requests = calloc(10, sizeof(Request));

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
    
    for (int i = 1; i < 10; i++) {
        proxy.pfds[i].fd = -1;
    }

    for (int i = 0; i < 10; i++) {
        proxy.requests[i].fd = -1;
    }

    for (int i = 0; i < 10; i++) {
        proxy.loaders[i].fd = -1;
    }

    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr));
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PROXY_PORT);  
    
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        return 1;
    } 

    if (listen(sockfd, 10) == -1) {
        return 1;
    }

    return 0;
}

//регистрируем прием входящих соединений,
int main() {
    if (initProxy()) {
        perror("initProxy failed");
        exit(1);
    }

    if (workLoop()) {
        perror("error in workloop");
        exit(2);
    }

    exit(EXIT_SUCCESS);
}