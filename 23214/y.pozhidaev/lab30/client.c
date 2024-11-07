#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

int main(){
    char *text = "HelLo WorLd aNd evERyone!";
    int sock;
    struct sockaddr_un sock_struct;
    static char* path = "choose_own_path.socket";

    memset(&sock_struct, 0, sizeof (sock_struct));
    sock_struct.sun_family = AF_UNIX;
    strncpy(sock_struct.sun_path, path, sizeof(sock_struct.sun_path) - 1);

    if((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket error");
        exit(1);
    }
    if ((connect(sock, (const struct sockaddr *)&sock_struct, sizeof(sock_struct))) == -1) {
        perror("connect error");
        close(sock);
        exit(2);
    }

    if (write(sock, text, strlen(text)+1) == -1){
        perror("write error");
        close(sock);
        exit(3);
    }

    close(sock);
    exit(0);
}
