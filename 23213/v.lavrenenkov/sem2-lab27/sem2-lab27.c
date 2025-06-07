#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 510
#define BACKLOG 128
#define BUFFER_SIZE 4096

typedef struct {
    int client_fd;
    int server_fd;
    char client_to_server_buf[BUFFER_SIZE];
    size_t c2s_len;   
    size_t c2s_sent;  
    char server_to_client_buf[BUFFER_SIZE];
    size_t s2c_len;
    size_t s2c_sent;
} connection_t;

int numConnections = 0;
int maxFdreached = 0;
connection_t connections[MAX_CONNECTIONS];

void close_connection(int i) {
    if (connections[i].client_fd >= 0) close(connections[i].client_fd);
    if (connections[i].server_fd >= 0) close(connections[i].server_fd);
    connections[i].client_fd = -1;
    connections[i].server_fd = -1;
    connections[i].c2s_len = connections[i].c2s_sent = 0;
    connections[i].s2c_len = connections[i].s2c_sent = 0;
    numConnections --;
    printf("%d\n", numConnections);
    maxFdreached = 0;
}

int create_listener_socket(int port) {
    int listener;
    struct sockaddr_in addr;
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(listener, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }
    return listener;
}

int connect_to_target(const char* host, const char* port) {
    struct addrinfo hints, *res, *rp;
    int sock;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return -1;
    }
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) continue;
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
            freeaddrinfo(res);
            numConnections++;
            return sock;
        }

        close(sock);
    }

    freeaddrinfo(res);
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <listen_port> <target_host> <target_port>\n", argv[0]);
        exit(1);
    }

    int listen_port = atoi(argv[1]);
    char* target_host = argv[2];
    char* target_port = argv[3];

    int listener = create_listener_socket(listen_port);

    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        connections[i].client_fd = -1;
        connections[i].server_fd = -1;
        connections[i].c2s_len = connections[i].c2s_sent = 0;
        connections[i].s2c_len = connections[i].s2c_sent = 0;
    }

    fd_set read_fds, write_fds;
    int max_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        if (maxFdreached == 0) {
            FD_SET(listener, &read_fds);
            max_fd = listener;
        }

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            if (connections[i].client_fd >= 0) {
                if (connections[i].c2s_len < BUFFER_SIZE)
                    FD_SET(connections[i].client_fd, &read_fds);
                if (connections[i].s2c_len < BUFFER_SIZE)
                    FD_SET(connections[i].server_fd, &read_fds);
                if (connections[i].c2s_sent < connections[i].c2s_len)
                    FD_SET(connections[i].server_fd, &write_fds);
                if (connections[i].s2c_sent < connections[i].s2c_len)
                    FD_SET(connections[i].client_fd, &write_fds);

                if (connections[i].client_fd > max_fd) max_fd = connections[i].client_fd;
                if (connections[i].server_fd > max_fd) max_fd = connections[i].server_fd;
            }
        }

        if (select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        

        if (FD_ISSET(listener, &read_fds)) {
            int assigned = -1;
            for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                if (connections[i].client_fd < 0) {
                    assigned = i;
                    break;
                }
            }
            if (assigned == -1) {
                maxFdreached = 1;
                printf("MAX CONNECTIONS\n");
                continue;
            }
            int client_fd = accept(listener, NULL, NULL);
            printf("New connection accepted: client_fd=%d\n", client_fd);
            if (client_fd >= 0) {
                int server_fd = connect_to_target(target_host, target_port);
                if (server_fd < 0) {
                    printf("Cant connect to target_host server_fd=%d\n", server_fd);
                    close(client_fd);
                    connections[assigned].client_fd = -1;
                    connections[assigned].server_fd = -1;
                } else {
                    printf("New connection accepted: server_fd=%d\n", server_fd);
                    connections[assigned].client_fd = client_fd;
                    connections[assigned].server_fd = server_fd;
                    printf("%d\n", numConnections);
                }
            }
        }

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            if (connections[i].client_fd < 0) continue;

            if (FD_ISSET(connections[i].client_fd, &read_fds)) {
                ssize_t len = recv(connections[i].client_fd,
                                   connections[i].client_to_server_buf + connections[i].c2s_len,
                                   BUFFER_SIZE - connections[i].c2s_len, 0);
                if (len <= 0) {
                    printf("Closing client_fd=%d (recv returned %zd) buffer size was %zd\n", connections[i].client_fd, len, BUFFER_SIZE - connections[i].c2s_len);
                    close_connection(i);
                    continue;
                }
                connections[i].c2s_len += len;
            }
            if (FD_ISSET(connections[i].server_fd, &write_fds)) {
                size_t remaining = connections[i].c2s_len - connections[i].c2s_sent;
                ssize_t sent = send(connections[i].server_fd,
                                    connections[i].client_to_server_buf + connections[i].c2s_sent,
                                    remaining, 0);
                if (sent <= 0) {
                    printf("Closing server_fd=%d (send returned %zd) buffer size was %zd\n", connections[i].server_fd, sent, remaining);
                    close_connection(i);
                    continue;
                }
                connections[i].c2s_sent += sent;
                if (connections[i].c2s_sent == connections[i].c2s_len) {
                    connections[i].c2s_sent = 0;
                    connections[i].c2s_len = 0;
                }
            }
            

            if (FD_ISSET(connections[i].server_fd, &read_fds)) {
                ssize_t len = recv(connections[i].server_fd,
                                   connections[i].server_to_client_buf + connections[i].s2c_len,
                                   BUFFER_SIZE - connections[i].s2c_len, 0);
                if (len <= 0) {
                    printf("Closing server_fd=%d (recv returned %zd) buffer size was %zd\n", connections[i].server_fd, len, BUFFER_SIZE - connections[i].s2c_len);
                    close_connection(i);
                    continue;
                }
                connections[i].s2c_len += len;
            }
            
            if (FD_ISSET(connections[i].client_fd, &write_fds)) {
                size_t remaining = connections[i].s2c_len - connections[i].s2c_sent;
                ssize_t sent = send(connections[i].client_fd,
                                    connections[i].server_to_client_buf + connections[i].s2c_sent,
                                    remaining, 0);
                if (sent <= 0) {
                    printf("Closing client_fd=%d (send returned %zd) buffer size was %zd\n", connections[i].server_fd, sent, remaining);
                    close_connection(i);
                    continue;
                }
                connections[i].s2c_sent += sent;
                if (connections[i].s2c_sent == connections[i].s2c_len) {
                    connections[i].s2c_sent = 0;
                    connections[i].s2c_len = 0;
                }
            }            
        }
    }
}
