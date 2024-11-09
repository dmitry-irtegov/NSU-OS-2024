#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define USERS 5

char *socket_path = "./socket";

void doUpper(char* buf, size_t length) {
    for (size_t i = 0; i < length; i++) buf[i] = toupper(buf[i]);
}

int create_socket() {
    int descriptor;
    if ((descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket failure");
        exit(-1);
    }

    struct sockaddr_un addr, client_addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(descriptor, (struct sockaddr* )&addr, sizeof(addr)) == -1) {
        perror("Bind failure");
        exit(-1);
    }

    if (listen(descriptor, USERS) == -1) {
        unlink(socket_path);
        perror("listen failure");
        exit(-1);
    }
    return descriptor;
}

int main() {
    int clientDescriptor, descriptor = create_socket();
    char buffer[BUFSIZ];

    if ((clientDescriptor = accept(descriptor, NULL, NULL)) == -1) {
        unlink(socket_path);
        perror("Accept failure");
        exit(-1);
    }
    
    size_t bufLen;
    while ((bufLen = read(clientDescriptor, buffer, BUFSIZ)) > 0)
    {
        doUpper(buffer, bufLen);
        write(STDOUT_FILENO, buffer, bufLen);
    }

    if (bufLen == -1) {
        perror("Read failure");
        unlink(socket_path);
        exit(-1);
    }
    unlink(socket_path);
    exit(0);
}
