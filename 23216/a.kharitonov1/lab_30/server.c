#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define buffer 512

void closeUnlinkOnFail(int soc, char* argv){
    close(soc);
    unlink(argv);
    exit(EXIT_FAILURE);
}
int main(int argc, char** argv) {
    if (argc < 2) {
        write(2,"you have to give a 1 socket name", strlen("you have to give a 1 socket name"));
        exit(EXIT_FAILURE);
    }
    char buf[buffer];
    ssize_t msglen;
    struct sockaddr_un addr;
    int soc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (soc == -1){
        perror("problem in socket");
        exit(EXIT_FAILURE);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, argv[1], sizeof(addr.sun_path)-1);
    if (bind(soc, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("problem in bind");
        close(soc);
        exit(EXIT_FAILURE);
    }
    if (listen(soc, 1) == -1) {
        write(2,"problem in listen", strlen("problem in listen"));
        closeUnlinkOnFail(soc, argv[1]);
    }
    puts("Wait for connection");
    int new = accept(soc, NULL, NULL);
    if (new == -1) {
        write(2,"problem in accept", strlen("problem in accept"));
        closeUnlinkOnFail(soc, argv[1]);
    }
    while((msglen = read(new, buf, buffer))>0){
        for(ssize_t i=0;i<msglen;i++){
            buf[i]=toupper(buf[i]);
        }
        if (write(1, buf, msglen) == -1){
            perror("problem in write");
            close(new);
            closeUnlinkOnFail(soc, argv[1]);
        }
    }
    if(msglen == -1){
        perror("problem in read");
        close(new);
        closeUnlinkOnFail(soc, argv[1]);
    }    
    close(new);
    close(soc);
    unlink(argv[1]);
    exit(EXIT_SUCCESS);
}