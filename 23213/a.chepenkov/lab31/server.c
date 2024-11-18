#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>

char* socket_path = "./socket";

int server_running = 1;

void to_uppercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

void handle_sigint(int sig) {
    server_running = 0;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    fd_set read_fds;
    int max_fd;
    

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening... Press Ctrl+C to stop.\n");

    max_fd = server_fd;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);

    while (server_running) {
        fd_set temp_fds = read_fds;

        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1) {
            if (errno != EINTR) {
                perror("select");
            }
            break;
        }

        if (FD_ISSET(server_fd, &temp_fds)) {
            if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
                perror("accept");
                continue;
            }
            printf("Client connected\n");
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        for (int i = 0; i <= max_fd; i++) {
            if (i != server_fd && FD_ISSET(i, &temp_fds)) {
                char buffer[BUFSIZ];
                ssize_t n = read(i, buffer, BUFSIZ - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    to_uppercase(buffer);
                    printf("Received from client %d: %s\n", i, buffer);
                }
                else if (n == 0) {
                    printf("Client %d disconnected\n", i);
                    close(i);
                    FD_CLR(i, &read_fds);
                }
                else {
                    perror("read");
                }
            }
        }
    }

    close(server_fd);
    unlink(socket_path);

    printf("\nServer stopped.\n");
    return 0;
}
