#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#include "macros.30.h"

int socketConnection(const char* socketname);
int interactWithServer(int socketDescriptor);

int main() {
    int socketDescriptor = socketConnection(PATH_TO_SOCKET);
    if (socketDescriptor == -1) {
        exit(EXIT_FAILURE);
    }

    if (interactWithServer(socketDescriptor) == -1) {
        close(socketDescriptor);
        exit(EXIT_FAILURE);
    }  

    exit(EXIT_SUCCESS);
}

int socketConnection(const char* socketname) {
    int socketDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketDescriptor == -1) {
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    if (strncpy(addr.sun_path, socketname, sizeof(addr.sun_path) - 1) == 0) {
        perror("cannot copy");
        close(socketDescriptor);
        return -1;
    }
    
    if (connect(socketDescriptor, (struct sockaddr_un *)&addr, sizeof(addr)) == -1) {
        perror("cannot connect");
        close(socketDescriptor);
        return -1;
    }

    return socketDescriptor;
}

int interactWithServer(int socketDescriptor) {
    char buffer[BUFFER_LENGTH];
    int bytesRead;

    while ((bytesRead = read(STDIN_FILENO, buffer, BUFFER_LENGTH)) > 0) {

        // it cannot be ZERO 'cause bytesRead is greater than zero
        if (write(socketDescriptor, buffer, bytesRead) < 0) {
            perror("cannot write");
            return -1;
        }
    }

    if (bytesRead == -1) {
        perror("cannot read");
    }

    return 0;
}
