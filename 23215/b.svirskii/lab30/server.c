#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include "server.h"
#define BUFF_SIZE (100)

extern const struct sockaddr_un sock_addr;
extern const int sock_addr_len;
extern int errno;

int socket_fd;

void close_all() {
    close(socket_fd);
    unlink(sock_addr.sun_path);
}

int main() {
    char buff[BUFF_SIZE + 1] = {0};
    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    
    if (socket_fd == -1) {
        fprintf(stderr, "socket() returned a error\n");
        exit(EXIT_FAILURE);
    }

    if (bind(socket_fd, (struct sockaddr*) &sock_addr, sock_addr_len) == -1
            && errno != EADDRINUSE) {
        fprintf(stderr, "socket binding ended with error\n");
        close_all();
        exit(EXIT_FAILURE);
    }
    
    if (listen(socket_fd, 1) == -1) {
        fprintf(stderr, "listen() ended with error\n");
        close_all();
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_un conn_sock_addr;
    socklen_t conn_sock_addr_len;
    int conn_sock_fd = accept(socket_fd, 
            (struct sockaddr*) &conn_sock_addr, &conn_sock_addr_len);
    
    if (conn_sock_fd == -1) {
        fprintf(stderr, "accept() ended with error\n");
        close_all();
        exit(EXIT_FAILURE);
    }
    
    ssize_t bytes_received;

    while(1) {
        if ((bytes_received = recv(conn_sock_fd, buff, BUFF_SIZE, 0)) == -1) {
            fprintf(stderr, "recv() ended with error\n");
            close_all();
            exit(EXIT_FAILURE);
        }

        if (bytes_received == 0) {
            break;
        }
    
        for (int i = 0; i < bytes_received; i++) {
            buff[i] = toupper(buff[i]);
        }

        buff[bytes_received] = 0;
        
        printf("%s", buff);
    }

    close_all();

    exit(EXIT_SUCCESS);
}
