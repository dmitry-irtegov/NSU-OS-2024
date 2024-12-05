#include <sys/socket.h> // sockets
#include <sys/un.h> // sockets constants
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(){
    char *text = "I hate my life\n";
    int client_fd;
    struct sockaddr_un sock_struct;
    static char* path = "mysock";

    memset(&sock_struct, 0, sizeof(sock_struct));
    sock_struct.sun_family = AF_UNIX;
    strncpy(sock_struct.sun_path, path, sizeof(sock_struct.sun_path) - 1);

    if((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket finished with error");
        return 1;
    }

    if ((connect(client_fd, (const struct sockaddr*) &sock_struct, sizeof(sock_struct))) == -1) {
        perror("connect finished with error");
        close(client_fd);
        return 1;
    }

    if (write(client_fd, text, strlen(text) + 1) == -1){
        perror("write finished with error");
        close(client_fd);
        return 1;
    }

    close(client_fd);
    return 0;
}
