#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <ctype.h>
int main(int argc, char *argv[]) {

    if (argc < 2){
        perror("missing socket name");
        exit(1);
    } 
    char *socket_name = argv[1];
    struct sockaddr_un addr;
    int fd,cl,rc;

    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(2);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path)-1);
    unlink(socket_name);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(3);
    }

    if (listen(fd, 1) == -1) {
        perror("listen error");
        exit(4);
    }

    if ( (cl = accept(fd, NULL, NULL)) == -1) {
        perror("accept error");
        exit(5);
    }
    char c[2] = {0};
    while ( (rc=read(cl, c, 1)) > 0) {
        c[0] = toupper(c[0]);
        if(write(1, c, 1) == -1){
            close(cl);
            perror("write error");
            exit(7);
        }
    }
    if(rc == -1){
        perror("read error");
        exit(4);
    }

    close(cl);
    return 0;
}