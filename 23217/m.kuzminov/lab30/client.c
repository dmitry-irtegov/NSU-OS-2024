#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int main() {
    int fd;
    char* socket_name = "./unix_socket";

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(1);
    }


    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_name);

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }
    
    char buff[BUFSIZ];
    
    ssize_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buff, sizeof(buff) - 1)) > 0) {
        if (write(fd, buff, bytes_read) == -1) {
            perror("write");
            break;
        }
    }

    if (bytes_read == -1) {
        perror("read");
    }
    close(fd);
}   
