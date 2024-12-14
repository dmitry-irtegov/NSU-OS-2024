#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
int main(int argc, char *argv[]) {

    if (argc < 2){
        perror("missing socket name");
        exit(1);
    } 
    if(strlen(argv[1]) > 107){
        perror("Too long socket name");
        exit(2);
    }
    char *socket_name = argv[1];
    struct sockaddr_un addr;
    int fd,cl,rc;
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(3);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path)-1);
    unlink(socket_name);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(4);
    }

    if (listen(fd, 1) == -1) {
        unlink(socket_name);
        perror("listen error");
        exit(5);
    }

    if ( (cl = accept(fd, NULL, NULL)) == -1) {
        unlink(socket_name);
        perror("accept error");
        exit(6);
    }
    char c[2] = {0};
    while ( (rc=read(cl, c, 1)) > 0) {
        c[0] = toupper(c[0]);
        if(write(1, c, 1) == -1){
            unlink(socket_name);
            perror("write error");
            exit(7);
        }
    }
    close(cl);
    unlink(socket_name);
    if(rc == -1){
        perror("read error");
        exit(8);
    }

    return 0;
}