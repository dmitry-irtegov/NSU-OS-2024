#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFFER_SIZE 512
char *socket_path = "socket";

int main() {

    int client_socket;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((client_socket = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {;
        perror("socket creation failure");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("socket binding failure");
        close(client_socket);
        exit(1);
    }

    printf("successful connection. (CTRL + D to terminate connection):\n");

    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1)) > 0) {

        if(bytes_read == -1) {
            perror("read failed");
            break;
        }

        if(buffer[bytes_read - 1] == '\n' ) {
            buffer[bytes_read - 1] = '\0';
        }

        if(write(client_socket, buffer, strlen(buffer)) < 0) {
            perror("sending data failure");
            break;
        }
        
    }

    close(client_socket);
    printf("client terminated\n");

    return 0;
}