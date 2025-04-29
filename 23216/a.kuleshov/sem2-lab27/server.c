#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#define MAX_CONNECTIONS 510
#define BUFFER_SIZE 4096

typedef struct {
    int client_fd;
    int server_fd;
    char client_buf[BUFFER_SIZE];
    char server_buf[BUFFER_SIZE];
    size_t client_buf_len;
    size_t server_buf_len;
    int active;
} connection_t;

static connection_t conns[MAX_CONNECTIONS] ={0};

int create_listen_socket(int port) {
    int sockfd;
    struct sockaddr_in addr = {0};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 128) < 0) {
        perror("listen");
        exit(1);
    }

    return sockfd;
}

int connect_to_server(const char *host, int port) {
    struct hostent *remote_host = gethostbyname(host);

    if (remote_host == NULL) {
        herror("gethostbyname");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *(struct in_addr*)remote_host->h_addr_list[0];

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int add_connection(int client_fd, int server_fd) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!conns[i].active) {
            conns[i].client_fd = client_fd;
            conns[i].server_fd = server_fd;
            conns[i].client_buf_len = 0;
            conns[i].server_buf_len = 0;
            conns[i].active = 1;
            return 0;
        }
    }
    return -1;
}

void close_connection(connection_t *conn) {
    close(conn->client_fd);
    close(conn->server_fd);
    conn->active = 0;
}

int add_connection_fds(struct pollfd *fds, int index, connection_t *conn) {
    fds[index].fd = conn->client_fd;
    fds[index].events = POLLIN | (conn->server_buf_len > 0 ? POLLOUT : 0);

    fds[index + 1].fd = conn->server_fd;
    fds[index + 1].events = POLLIN | (conn->client_buf_len > 0 ? POLLOUT : 0);

    return 2;
}

void handle_read_events(struct pollfd *fds, int index, connection_t *conn) {
    if (fds[1 + index * 2].revents & POLLIN) {
        ssize_t n = read(conn->client_fd, conn->client_buf + conn->client_buf_len, BUFFER_SIZE - conn->client_buf_len);
        if (n > 0) {
            conn->client_buf_len += n;
        } else {
            close_connection(conn);
            return;
        }
    }

    if (fds[1 + index * 2 + 1].revents & POLLIN) {
        ssize_t n = read(conn->server_fd, conn->server_buf + conn->server_buf_len, BUFFER_SIZE - conn->server_buf_len);
        if (n > 0) {
            conn->server_buf_len += n;
        } else {
            close_connection(conn);
        }
    }
}

void handle_write_events(struct pollfd *fds, int index, connection_t *conn) {
    if (fds[1 + index * 2].revents & POLLOUT && conn->server_buf_len > 0) {
        ssize_t n = write(conn->client_fd, conn->server_buf, conn->server_buf_len);
        if (n > 0) {
            memmove(conn->server_buf, conn->server_buf + n, conn->server_buf_len - n);
            conn->server_buf_len -= n;
        } else {
            close_connection(conn);
            return;
        }
    }

    if (fds[1 + index * 2 + 1].revents & POLLOUT && conn->client_buf_len > 0) {
        ssize_t n = write(conn->server_fd, conn->client_buf, conn->client_buf_len);
        if (n > 0) {
            memmove(conn->client_buf, conn->client_buf + n, conn->client_buf_len - n);
            conn->client_buf_len -= n;
        } else {
            close_connection(conn);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <listen_port> <target_host> <target_port>\n", argv[0]);
        exit(1);
    }

    int listen_port = atoi(argv[1]);
    const char *target_host = argv[2];
    int target_port = atoi(argv[3]);

    int listen_fd = create_listen_socket(listen_port);
    struct pollfd fds[1 + MAX_CONNECTIONS * 2];

    int nfds = 0;

    // Add listen socket
    fds[nfds].fd = listen_fd;
    fds[nfds].events = POLLIN;

    while (1) {
        nfds = 1;

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (conns[i].active) {
                nfds += add_connection_fds(fds, nfds, &conns[i]);
            }
        }

        int ready = poll(fds, nfds, -1);
        if (ready < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            exit(1);
        }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
            if (client_fd >= 0) {
                int server_fd = connect_to_server(target_host, target_port);
                if (server_fd >= 0) {
                    if (add_connection(client_fd, server_fd) == 0) {
                        printf("New connection established!\n");
                    } else {
                        close(client_fd);
                        close(server_fd);
                    }
                } else {
                    close(client_fd);
                }
            }
        }

        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (conns[i].active) {
                handle_read_events(fds, i, &conns[i]);
                if (!conns[i].active) continue;
                handle_write_events(fds, i, &conns[i]);
            }
        }
    }

    return 0;
}
