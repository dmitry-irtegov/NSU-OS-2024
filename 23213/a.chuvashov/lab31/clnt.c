#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>

char *socket_path = "./socket";

int main(int argc, char** argv) {
    int datafd = STDIN_FILENO;
    if (argc == 2) {
        if ((datafd = open(argv[1], O_RDONLY)) == -1) {
            perror("Open failure");
            exit(-1);
        }
    }

    int descriptor, client_descriptor;
    if ((descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket failure");
        exit(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    
    if (connect(descriptor, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("Connect failure");
        exit(-1); 
    }
    
    char buffer[BUFSIZ];
    ssize_t bytes;
    while((bytes = read(datafd, buffer, sizeof(buffer))) > 0) {
        write(descriptor, buffer, bytes);
    }

    exit(0);
}
