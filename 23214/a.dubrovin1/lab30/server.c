#include <sys/socket.h> // sockets
#include <sys/un.h> // sockets constants
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int main(){
    char text[BUFSIZ];
    char* path = "mysock";
    int server_fd, client_fd;
    struct sockaddr_un sock_struct;

    memset(&sock_struct, 0, sizeof (sock_struct));
    sock_struct.sun_family = AF_UNIX;
    strncpy(sock_struct.sun_path, path, sizeof(sock_struct.sun_path) - 1);

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket finished with error");
        return 1;
    }

    if ((bind(server_fd, (struct sockaddr*) &sock_struct, sizeof(sock_struct))) == -1){
        perror("bind fimished with error");
        close(server_fd);
        return 1;
    }

    if ((listen(server_fd, 1)) == -1){
        perror("listen finished with error");
        close(server_fd);
        unlink(sock_struct.sun_path);
        return 1;
    }

    if ((client_fd = accept(server_fd, 0, 0)) == -1){
        perror("accept finished with error");
        close(server_fd);
        unlink(sock_struct.sun_path);
        return 1;
    }

    int readsize;
    while ((readsize = read(client_fd, text, BUFSIZ - 1)) != 0){
        if(readsize == -1){
            perror("read finished with error");
            close(server_fd);
            close(client_fd);
            unlink(sock_struct.sun_path);
            return 1;
        }

        text[readsize] = '\0';
        for (int i = 0; i < readsize; i++){
            text[i] = toupper(text[i]);
        }

        printf("%s", text);
    }

    close(server_fd);
    close(client_fd);
    unlink(sock_struct.sun_path);
    return 0;
}
