#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


void main(void) {
    char *name_file = "endpoint";
    struct sockaddr_un adress;
    adress.sun_family=AF_UNIX;
    char buffer[5];
    buffer[4]='\0';
    strncpy(adress.sun_path, name_file, sizeof(adress.sun_path));
    int server_fd;
    int client_fd;
    int char_read;
    if((server_fd=socket(AF_UNIX, SOCK_STREAM, 0))<0){
        perror("socket initialization error");
        exit(1);
    }
    if(bind(server_fd, (struct sockaddr *)&adress, sizeof(adress))<0){
        perror("socked bind error");
        exit(2);
    }
    if(listen(server_fd, 3)<0){
        close(server_fd);
        unlink(name_file);
        perror("listen error");
        exit(3);
    }
    if((client_fd=accept(server_fd, NULL, NULL))==-1){
        close(server_fd);
        unlink(name_file);
        perror("accept error");
        exit(4);
    }
    while((char_read=read(client_fd, buffer, 4))!=0){
        if(char_read==-1){
            close(client_fd);
            close(server_fd);
            unlink(name_file);
            perror("read error");
            exit(5);
        }
        for(int i = 0;i<char_read;i++){
            buffer[i]=toupper(buffer[i]);
        }
        printf("%s", buffer);
        fflush(stdout);
    }
    close(client_fd);
    close(server_fd);
    unlink(name_file);
    exit(0);

}