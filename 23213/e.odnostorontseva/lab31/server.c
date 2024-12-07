#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

#define CONNECTIONS_LIMIT 10
#define socket_path "./socket"

int server_running = 1;
void handle_sigint(int sig) {
    server_running = 0;
    printf("Signal %d\n", sig);
}


int main() {
    unlink(socket_path);
    int server_d = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_d == -1) {
        perror("socket error");
        exit(1);
    }
   
    struct sockaddr_un addr = {0};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(server_d, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(server_d);
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    if (listen(server_d, CONNECTIONS_LIMIT) == -1) {
        perror("listen error");
        close(server_d);
        unlink(socket_path);
        exit(1);
    }

    printf("Server is listening ...\n");

    struct pollfd clients_fds[CONNECTIONS_LIMIT + 1];
    int num_fds = 1;

    clients_fds[0].fd = server_d;
    clients_fds[0].events = POLLIN;

    while (server_running) {
        if ((poll(clients_fds, num_fds, -1)) == -1) {
            if (errno == EINTR) continue;
            perror("poll error");
            break;
        }

        for (int i = 0; i < num_fds; i++) {
            if (clients_fds[i].fd == -1) {
                continue;
            }
            if (clients_fds[i].revents & POLLIN) {
                if (clients_fds[i].fd == server_d) {
                    int client_fd = accept(server_d, NULL, NULL);
                    if (client_fd == -1) {
                        perror("accept error");
                        continue;
                    }

                    printf("Client %d connected.\n", client_fd);

                    if (num_fds >= CONNECTIONS_LIMIT) {
                        printf("Too many connections...\n");
                        printf("Closing connection...\n");
                        close(client_fd);
                    }

                
                    clients_fds[num_fds].fd = client_fd;
                    clients_fds[num_fds].events = POLLIN;
                    num_fds += 1;

                } else {
                    char buffer[BUFSIZ];

                    int bytes_read = read(clients_fds[i].fd, buffer, sizeof(buffer));

                    if (bytes_read > 0) {
                        if(buffer[bytes_read - 1] == '\n') {
                            buffer[bytes_read - 1] = '\0';
                        }
                        for (int i = 0; i < bytes_read; i++) {
                            buffer[i] = toupper(buffer[i]);
                        }
                        printf("Received from client %d: %s\n", clients_fds[i].fd, buffer);

                    } else if (bytes_read == 0) {
                        printf("Client %d disconnected.\n", clients_fds[i].fd);
                        close(clients_fds[i].fd);
                        clients_fds[i].fd = -1;
                        num_fds -= 1;

                    } else {
                        perror("read error");
                        close(clients_fds[i].fd);
                        clients_fds[i].fd = -1;
                    }
                }                
            }        
        }
    }

    close(server_d);
    unlink(socket_path);
    printf("Server stopped.\n");
    return 0;
}

