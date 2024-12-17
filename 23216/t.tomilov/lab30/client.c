#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc < 2){
        fprintf(stderr, "ERROR: wrong arg format to start! Try %s <socet_name>", argv[0]);
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
        unlink(argv[1]);
        exit(EXIT_FAILURE);
    }

    if (write(sockfd, "I`m just a message! What do you want from me?\n", strlen("I`m just a message! What do you want from me?\n")) == -1){
        perror("ERROR: failde in write!");
        unlink(argv[1]);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    unlink(argv[1]);
    exit(EXIT_SUCCESS);
}