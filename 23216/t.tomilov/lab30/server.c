#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 256

int main(int argc, char** argv){
    if (argc < 2){
        fprintf(stderr, "ERROR: wrong arg format to start! Try %s <socet_name>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("ERROR: failed in socket!");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        perror("ERROR: failed in bind!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (listen(sockfd, 1) == -1){
        perror("ERROR: failed in listen");
        close(sockfd);
        unlink(argv[1]);
        exit(EXIT_FAILURE);
    }

    int clientSockfd = accept(sockfd, NULL, NULL);
    if (clientSockfd == -1){
        perror("ERROR: failed in accept!");
        close(sockfd);
        unlink(argv[1]);
        exit(EXIT_FAILURE);
    }

    char str[BUFSIZE];
    int len;
    while((len = read(clientSockfd, str, BUFSIZE)) > 0){
        char upperStr[BUFSIZE];
        for (int i = 0; i < len; i++){
            upperStr[i] = toupper(str[i]);
        }
        if (write(1, upperStr, len) == -1){
            perror("ERROR: failed to write!");
            close(sockfd);
            close(clientSockfd);
            unlink(argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    close(sockfd);
    close(clientSockfd);
    unlink(argv[1]);
    exit(EXIT_SUCCESS);
}
    