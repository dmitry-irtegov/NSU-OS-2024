#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "proxy.h"
#include "cache.h"

#define PORT 65430
#define MAX_CLIENTS 100

int main() {
    int server_fd, client_fd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    struct pollfd fds[MAX_CLIENTS];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get failed");
        exit(EXIT_FAILURE);
    }
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Proxy server listening on port %d\n", PORT);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    for (i = 1; i < MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    while (1) {
        activity = poll(fds, MAX_CLIENTS, -1);

        if (activity < 0) {
            perror("poll error");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                continue;
            }

            flags = fcntl(client_fd, F_GETFL, 0);
            if (flags == -1) {
                perror("fcntl get failed");
                close(client_fd);
                continue;
            }
            if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                perror("fcntl set failed");
                close(client_fd);
                continue;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   client_fd, client_ip, ntohs(address.sin_port));

            for (i = 1; i < MAX_CLIENTS; i++) {
                if (fds[i].fd < 0) {
                    fds[i].fd = client_fd;
                    fds[i].events = POLLIN;
                    break;
                }
            }
            if (i == MAX_CLIENTS) {
                printf("Too many clients connected\n");
                close(client_fd);
            }
        }

        for (i = 1; i < MAX_CLIENTS; i++) {
            if (fds[i].fd > 0 && (fds[i].revents & POLLIN)) {
                if (handle_client_request(fds[i].fd) == 0) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    printf("Connection closed\n");

                } else {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    printf("Connection forcibly closed due to error\n");
                }
            }
        }
    }

    return 0;
}