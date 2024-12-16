#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

char* path = "./soc";

int main(int argc, char** argv) {
    int fd;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return 1;
    }
    struct sockaddr_un adr;
    memset(&adr, 0, sizeof(adr));
    adr.sun_family = AF_UNIX;
    strncpy(adr.sun_path, path, sizeof(adr.sun_path)-1);
    if (connect(fd, (struct sockaddr*)&adr, sizeof(adr)) == -1) {
        perror("connect error");
        return 1;
    }
    ssize_t readed;
    char buf[BUFSIZ];

    while((readed = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (write(fd, buf, readed) != readed) {
            if (readed > 0){
                fprintf(stderr,"partial write");
            } else {
                perror("write error");
                return 1;
            }
        }
    }
    return 0;
}
