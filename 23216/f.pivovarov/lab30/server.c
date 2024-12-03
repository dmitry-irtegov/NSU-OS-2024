#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#include "macros.30.h"

int socketInitialization(const char *pathToSocket, int queueConnectionLength);
int connectionProccessing(int socketDesctiptor);
void socketUnlinkClose(int socketDescriptor, const char *pathToSocket);


int main() {
    // get fd of socket
    int socketDescriptor = socketInitialization(PATH_TO_SOCKET, 1);
    if (socketDescriptor == -1) {
        exit(EXIT_FAILURE);
    }
    // get new fd for new connection
    int newScoketDesctiptor = accept(socketDescriptor, NULL, NULL);
    if (newScoketDesctiptor == -1) {
        socketUnlinkClose(socketDescriptor, PATH_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
    // proccess connection
    if (connectionProccessing(newScoketDesctiptor)) {
        socketUnlinkClose(socketDescriptor, PATH_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
    socketUnlinkClose(socketDescriptor, PATH_TO_SOCKET);
    exit(EXIT_SUCCESS);
}

int socketInitialization(const char *pathToSocket, int queueConnectionLength) {
    // Create socket fd
    int socketDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketDescriptor == -1) {
        perror("Cannot create socketFD");
        return -1;
    }
    // Set config of socket address
    struct sockaddr_un addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    if (strncpy(addr.sun_path, PATH_TO_SOCKET, sizeof(addr.sun_path) - 1) == NULL) {
        perror("cannot copy path to socket");
        close(socketDescriptor);
        return -1;
    }
    // unlink for cases, when socket already binded with this name
    unlink(PATH_TO_SOCKET);
    // bind and listen
    if (bind(socketDescriptor, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("cannot bind");
        close(socketDescriptor);
        return -1;
    }
    if (listen(socketDescriptor, queueConnectionLength) == -1) {
        perror("cannot listen");
        socketUnlinkClose(socketDescriptor, pathToSocket);
        return -1;
    }

    return socketDescriptor;
}

int connectionProccessing(int socketDesctiptor) {
    char buffer[BUFFER_LENGTH];
    int bytesRead;

    while((bytesRead = read(socketDesctiptor, buffer, BUFFER_LENGTH)) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        
        if (write(STDOUT_FILENO, buffer, bytesRead) == -1) {
            perror("cannot write");
            close(socketDesctiptor);
            return -1;
        }
    }

    close(socketDesctiptor);
    if (bytesRead == -1) {
            perror("cannot read");
            return -1;
    }
    return 0;
}

void socketUnlinkClose(int socketDescriptor, const char *pathToSocket) {
    unlink(pathToSocket);
    close(socketDescriptor);
}
