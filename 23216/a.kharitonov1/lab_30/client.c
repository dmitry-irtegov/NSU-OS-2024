#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#define buffer 512

void signalHandler(){
    write(2,"lost connection with sercer", strlen("lost connection with sercer"));
    _exit(EXIT_FAILURE);
}

int main(int argc, char** argv){
    if (argc < 2) {
        write(2,"you have to give a 1 socket name", strlen("you have to give a 1 socket name"));
        exit(EXIT_FAILURE);
    }
    if (signal(SIGPIPE, signalHandler) == SIG_ERR) {
        perror("problem in signal");
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
    if (connect(soc, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        perror("problem in connect");
        close(soc);
        exit(EXIT_FAILURE);
    }
    puts("Use CTRL-D for the end");
    while ((msglen = read(0, buf, buffer)) > 0) {
        if (write(soc, buf, msglen) <= 0) {
            perror("workWithConnection failed");
            close(soc);
            exit(EXIT_FAILURE);
        }
    }
    if (msglen < 0) {
        perror("problem in read");
        close(soc);
        exit(EXIT_FAILURE);
    }
    close(soc);
    exit(EXIT_SUCCESS);
}