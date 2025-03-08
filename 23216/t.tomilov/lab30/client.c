#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define BUFSIZE 256

void newSigReact(){
    perror("Lost connection with server!");
    _exit(EXIT_FAILURE);
}

int main(int argc, char** argv){
    if (argc < 2){
        fprintf(stderr, "ERROR: wrong arg format to start! Try %s <socet_name>", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (signal(SIGPIPE, newSigReact) == SIG_ERR) {
        perror("ERROR: failde in signal!");
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("ERROR: failed in socket!");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);
    
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        perror("ERROR: failed in connect!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char str[BUFSIZE];
    size_t n;
    while((n = read(0, str, BUFSIZE)) > 0){
        if (write(sockfd, str, n) == -1){
            perror("ERROR: failde in write!");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    if((int)n == -1){
        perror("ERROR: failde in read!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}