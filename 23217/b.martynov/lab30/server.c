#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFET_SIZE 1024

char* socket_path = "./socket_path";

int flag = 0;

void SIGINTer(int sigNum) {
    unlink(socket_path);
    exit(EXIT_SUCCESS);
}

int main() {
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1){
        perror("Error in server socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
       perror("Error in bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, SIGINTer) == SIG_ERR) {
        perror("Error whiiile set SIGINT handler");
        close(server_socket);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket, 1) == -1) {
        perror("Error in listen");
        close(server_socket);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Error in accept");
        close(server_socket);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    char bufet[BUFET_SIZE] = "";
    ssize_t cnt_bytes = 0;
    while ((cnt_bytes = read(client_socket, bufet, BUFET_SIZE)) > 0) {
        for (int i = 0; i < cnt_bytes; i++) {
            bufet[i] = toupper(bufet[i]);
        }
        write(STDOUT_FILENO, bufet, cnt_bytes);
    }
    if (cnt_bytes == -1) {
        perror("Error in reading");
        close(client_socket);
        close(server_socket);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }
    close(client_socket);
    close(server_socket);
    unlink(socket_path);
    return 0;
}
