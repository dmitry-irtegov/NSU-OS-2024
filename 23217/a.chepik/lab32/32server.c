#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <aio.h>
#include <signal.h>
#include <errno.h>

#define BACKLOG 17

const char* socket_path = "my_socket";

void my_unlink() {
    unlink(socket_path);
    printf("Unlink is done.\n");
}

void handler(int signum) {
    printf("Server interrupted by SIGINT signal.\n");
    exit(-1);
}

typedef struct {
    struct aiocb aio_cb;
    int fd;
    char buffer[BUFSIZ];
} aio_client;

aio_client clients[BACKLOG];

void close_client(aio_client* client) {
    if (client->fd != -1) {
        close(client->fd);
        client->fd = -1;
    }
}

void start_async_read(aio_client* client) {
    struct aiocb* cb = &client->aio_cb;
    memset(cb, 0, sizeof(struct aiocb));
    cb->aio_fildes = client->fd;
    cb->aio_nbytes = BUFSIZ;
    cb->aio_buf = client->buffer;
    cb->aio_offset = 0;

    if (aio_read(cb) == -1) {
        printf("aio_read() failed.\n");
        close_client(client);
    }
}

void aio_handling(aio_client* client) {
    struct aiocb* cb = &client->aio_cb;

    if (aio_error(cb) == 0) {
        ssize_t bytes_read = aio_return(cb);

        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                printf("%c", toupper(client->buffer[i]));
            }

            start_async_read(client);
        }

        else {
            if (bytes_read < 0) {
                printf("server aio_return() failed.\n");
            }

            close_client(client);
        }
    }
}

int main() {
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        printf("server socket() failed.\n");
        exit(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("bind() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    if (atexit(my_unlink) != 0) {
        printf("atexit() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    signal(SIGINT, handler);

    if (listen(socket_fd, 1) != 0) {
        printf("listen() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    for (int i = 0; i < BACKLOG; i++) {
        clients[i].fd = -1;
    }

    while (1) {
        int client_fd = accept(socket_fd, NULL, NULL);

        if (client_fd == -1) {
            printf("accept() failed.\n");
            continue;
        }

        int found_slot = 0;

        for (int i = 0; i < BACKLOG; i++) {
            if (clients[i].fd == -1) {
                clients[i].fd = client_fd;
                start_async_read(&clients[i]);
                found_slot = 1;
                break;
            }
        }

        if (!found_slot) {
            close(client_fd);
        }

        struct aiocb* aios[BACKLOG];
        int n = 0;

        for (int i = 0; i < BACKLOG; i++) {
            if (clients[i].fd != -1) {
                int err = aio_error(&clients[i].aio_cb);
                if (err == EINPROGRESS) {
                    aios[n++] = &clients[i].aio_cb;
                }

                else {
                    printf("Client status not EINPROGRESS.\n");
                    close_client(&clients[i]);
                }
            }
        }

        if (n > 0) {
            int res = aio_suspend(aios, n, NULL);

            if (res == -1) {
                printf("aio_suspend() failed.\n");
                close(socket_fd);
                exit(-1);
            }
        }

        for (int i = 0; i < BACKLOG; i++) {
            if (clients[i].fd != -1) {
                aio_handling(&clients[i]);
            }
        }
    }

    exit(0);
}
