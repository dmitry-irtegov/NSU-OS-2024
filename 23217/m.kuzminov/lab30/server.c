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


    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        exit(1);
    }


    if (listen(fd, 1) == -1) {
        perror("listen failed");
        exit(1);
    }



    int acfd;
    if ((acfd = accept(fd, NULL, NULL)) == -1) {
        perror("accept failed");
        exit(1);
    }
    char buffer[BUFSIZ];
    ssize_t len;
    while ((len = read(acfd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < len; i++) {
            printf("%c",toupper(buffer[i]));
        }
    }

    close(acfd);
    close(fd);

    unlink(socket_name);

    return 0;
}
