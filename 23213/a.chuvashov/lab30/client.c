#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int descriptor;
    if ((descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket failure");
        exit(-1);
    }
    int server_descriptor;
    char buffer[BUFSIZ];
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "./socket", sizeof(addr.sun_path)-1);
    

    if ((connect(descriptor, (struct sockaddr* )&addr, sizeof(addr))) == -1) {
        perror("Connect failure");
        exit(-1);
    }

    size_t bytes;
    while((bytes = read(0, buffer, BUFSIZ)) > 0) {
        if (write(descriptor, buffer, bytes) == -1) {
            perror("Write failure");
            exit(-1);
        } 
    }

    if (bytes == -1) {
        perror("Read failure");
        exit(-1);
    }

    exit(0);
}
