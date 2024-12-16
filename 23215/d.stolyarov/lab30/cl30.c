#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    int fd,rc;

    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(3);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(4);
    }
    char buf[100] = {0};
    while( (rc=read(0, buf, 100)) > 0) {
        
        if (write(fd, buf, rc) != rc) {
            perror("write error");
            exit(5);
        }
    }
    if(rc == -1){
        perror("read error");
        exit(6);
    }
    return 0;
}