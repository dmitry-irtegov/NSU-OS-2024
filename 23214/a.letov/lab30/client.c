#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> 

void main(void){
    int client_fd;
    char *name_file="endpoint";
    struct sockaddr_un adress;
    adress.sun_family=AF_UNIX;
    strncpy(adress.sun_path, name_file, sizeof(adress.sun_path));
    char message[31]="TestInGerereakwebrawshahaha";
    if((client_fd=socket(AF_UNIX, SOCK_STREAM, 0))<0){
        perror("client socket initialization error");
        exit(1);
    }
    if(connect(client_fd, (struct sockaddr*)&adress, sizeof(adress))<0){
        close(client_fd);
        perror("connect error");
        exit(2);
    }
    if(write(client_fd, message, 31)==-1){
        close(client_fd);
        perror("write error");
        exit(3);
    }
    close(client_fd);
    exit(0);
}