#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#define BUFFER_SIZE 255

int initialize_address(const char *ip_address, const char *port_number, struct sockaddr_in6 *address) {
    memset(address, 0, sizeof(*address));
    address->sin6_family = AF_INET6;
    address->sin6_port = htons(atoi(port_number));
    return inet_pton(AF_INET6, ip_address, &address->sin6_addr);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IPv6 address> <port>\n", argv[0]);
        return 1;
    }

    struct sockaddr_in6 server_address;
    if (initialize_address(argv[1], argv[2], &server_address) < 0) {
        fprintf(stderr, "Invalid address\n");
        return 1;
    }

    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect");
        close(socket_fd);
        return 1;
    }

    printf("Connected to the server\n");

    fd_set read_fds;
    char buffer[BUFFER_SIZE];

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds); 
        FD_SET(socket_fd, &read_fds); 

        int max_fd = socket_fd > 0 ? socket_fd : 0;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(socket_fd, &read_fds)) {
            char response[BUFFER_SIZE];
            ssize_t bytes_received = recv(socket_fd, response, BUFFER_SIZE - 1, 0);
            if (bytes_received < 0) {
                perror("recv");
                break;
            } else if (bytes_received == 0) {
                printf("Server closed the connection\n");
                break;
            }

            response[bytes_received] = '\0';
            printf("Received from server: %s\n", response);
        }


        if (FD_ISSET(0, &read_fds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0; 

            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            ssize_t bytes_sent = send(socket_fd, buffer, strlen(buffer), 0);
            if (bytes_sent < 0) {
                perror("send");
                break;
            }
        }
    }

    close(socket_fd);
    return 0;
}

