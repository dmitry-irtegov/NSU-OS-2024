#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>

#define MAX_CONNECTIONS 510
#define BUFFER_SIZE 1024

int local_port;
char *remote_host;
int remote_port;

typedef struct twin {
    int client;
    int server;
    char client_to_server_buf[BUFFER_SIZE];
    int client_to_server_len;
    char server_to_client_buf[BUFFER_SIZE];
    int server_to_client_len;
} twin;

int create_listen_socket();
void proxy_server(int listen_fd);
void handle_new_connection(int listen_fd, twin *connections, int *num_connections, struct pollfd *pfds, int *num_pfds);
void handle_existing_connections(twin *connections, int *num_connections, struct pollfd *pfds, int *num_pfds);
void close_connection(twin *connections, int *num_connections, struct pollfd *pfds, int *num_pfds, int j);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <P> <N> <P`>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    local_port = atoi(argv[1]);
    remote_host = argv[2];
    remote_port = atoi(argv[3]);

    int listen_fd = create_listen_socket();
    proxy_server(listen_fd);
    close(listen_fd);
    exit(EXIT_SUCCESS);
}

int create_listen_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(local_port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, MAX_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return fd;
}

void proxy_server(int listen_fd) {
    twin connections[MAX_CONNECTIONS];
    struct pollfd pfds[MAX_CONNECTIONS * 2 + 1];
    int num_connections = 0;
    int num_pfds = 0;

    pfds[0].fd = listen_fd;
    pfds[0].events = POLLIN;
    num_pfds = 1;

    while (1) {
        if (poll(pfds, num_pfds, -1) < 0) {
            perror("poll");
            continue;
        }

        if (pfds[0].revents & POLLIN) {
            handle_new_connection(listen_fd, connections, &num_connections, pfds, &num_pfds);
        }

        handle_existing_connections(connections, &num_connections, pfds, &num_pfds);
    }
}

void handle_new_connection(int listen_fd, twin *connections, int *num_connections, struct pollfd *pfds, int *num_pfds) {
    if (*num_connections >= MAX_CONNECTIONS) {
        fprintf(stderr, "Max connections reached\n");
        return;
    }

    int client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        close(client_fd);
        return;
    }

    struct hostent *he = gethostbyname(remote_host);
    if (he == NULL) {
        herror("gethostbyname");
        close(server_fd);
        close(client_fd);
        return;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_port);
    addr.sin_addr = *(struct in_addr*)he->h_addr_list[0];

    if (connect(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(server_fd);
        close(client_fd);
        return;
    }
   
    connections[*num_connections].client = client_fd;
    connections[*num_connections].server = server_fd;
    connections[*num_connections].client_to_server_len = 0;
    connections[*num_connections].server_to_client_len = 0;
    (*num_connections)++;

    pfds[*num_pfds].fd = client_fd;
    pfds[*num_pfds].events = POLLIN;
    (*num_pfds)++;
    pfds[*num_pfds].fd = server_fd;
    pfds[*num_pfds].events = POLLIN;
    (*num_pfds)++;
}


void handle_existing_connections(twin *connections, int *num_connections, struct pollfd *pfds, int *num_pfds) {
    for (int i = 0; i < *num_connections; i++) {
        int client_fd = connections[i].client;
        int server_fd = connections[i].server;

        int client_pfd_idx = -1, server_pfd_idx = -1;
        for (int k = 1; k < *num_pfds; k++) {
            if (pfds[k].fd == client_fd) client_pfd_idx = k;
            if (pfds[k].fd == server_fd) server_pfd_idx = k;
        }

        if (client_pfd_idx != -1 && (pfds[client_pfd_idx].revents & POLLIN)) {
            int space = BUFFER_SIZE - connections[i].client_to_server_len;
            if (space > 0) {
                char temp_buf[BUFFER_SIZE];
                int bytes_read = read(client_fd, temp_buf, space);
                if (bytes_read <= 0) {
                    close_connection(connections, num_connections, pfds, num_pfds, i);
                    i--;
                    continue;
                }
                memcpy(connections[i].client_to_server_buf + connections[i].client_to_server_len, temp_buf, bytes_read);
                connections[i].client_to_server_len += bytes_read;
                pfds[server_pfd_idx].events |= POLLOUT;
            }
        }

        if (server_pfd_idx != -1 && (pfds[server_pfd_idx].revents & POLLIN)) {
            int space = BUFFER_SIZE - connections[i].server_to_client_len;
            if (space > 0) {
                char temp_buf[BUFFER_SIZE];
                int bytes_read = read(server_fd, temp_buf, space);
                if (bytes_read <= 0) {
                    close_connection(connections, num_connections, pfds, num_pfds, i);
                    i--;
                    continue;
                }
                memcpy(connections[i].server_to_client_buf + connections[i].server_to_client_len, temp_buf, bytes_read);
                connections[i].server_to_client_len += bytes_read;
                pfds[client_pfd_idx].events |= POLLOUT;
            }
        }

        if (server_pfd_idx != -1 && (pfds[server_pfd_idx].revents & POLLOUT)) {
            if (connections[i].client_to_server_len > 0) {
                int bytes_written = write(server_fd, connections[i].client_to_server_buf, connections[i].client_to_server_len);
                if (bytes_written <= 0) {
                    close_connection(connections, num_connections, pfds, num_pfds, i);
                    i--;
                    continue;
                }
                connections[i].client_to_server_len -= bytes_written;
                memmove(connections[i].client_to_server_buf, connections[i].client_to_server_buf + bytes_written, connections[i].client_to_server_len);
                if (connections[i].client_to_server_len == 0) {
                    pfds[server_pfd_idx].events &= ~POLLOUT;
                }
            }
        }

        if (client_pfd_idx != -1 && (pfds[client_pfd_idx].revents & POLLOUT)) {
            if (connections[i].server_to_client_len > 0) {
                int bytes_written = write(client_fd, connections[i].server_to_client_buf, connections[i].server_to_client_len);
                if (bytes_written <= 0) {
                    close_connection(connections, num_connections, pfds, num_pfds, i);
                    i--;
                    continue;
                }
                connections[i].server_to_client_len -= bytes_written;
                memmove(connections[i].server_to_client_buf, connections[i].server_to_client_buf + bytes_written, connections[i].server_to_client_len);
                if (connections[i].server_to_client_len == 0) {
                    pfds[client_pfd_idx].events &= ~POLLOUT;
                }
            }
        }
    }
}

void close_connection(twin *connections, int *num_connections, struct pollfd *pfds, int *num_pfds, int j) {
    close(connections[j].client);
    close(connections[j].server);

    for (int k = *num_pfds - 1; k >= 1; k--) {
        if (pfds[k].fd == connections[j].client || pfds[k].fd == connections[j].server) {
            pfds[k] = pfds[*num_pfds - 1];
            (*num_pfds)--;
        }
    }

    connections[j] = connections[*num_connections - 1];
    (*num_connections)--;
}
