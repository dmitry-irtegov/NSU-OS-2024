#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

void clear(int sock, struct sockaddr_un* sock_struct){
    close(sock);
    unlink(sock_struct->sun_path);
}

int main(){
    char text[BUFSIZ];
    static char* path = "choose_own_path.socket";
    int sock, accepted_sock, size;
    struct sockaddr_un sock_struct;

    memset(&sock_struct, 0, sizeof (sock_struct));
    sock_struct.sun_family = AF_UNIX;
    strncpy(sock_struct.sun_path, path, sizeof(sock_struct.sun_path) - 1);

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0) ) == -1){
        perror("socket error");
        exit(1);
    }
    if ((bind(sock, (struct sockaddr *) &sock_struct, sizeof(sock_struct))) == -1){
        perror("bind error");
        close(sock);
        exit(2);
    }
    if ((listen(sock, 2)) == -1){
        perror("listen error");
        clear(sock, &sock_struct);
        exit(3);
    }
    if ((accepted_sock = accept(sock, 0, 0)) == -1){
        perror("accept error");
        clear(sock, &sock_struct);
        exit(4);
    }
    while ((size = read(accepted_sock, text, BUFSIZ)) != 0){
        if(size == -1){
            perror("read error");
            clear(sock, &sock_struct);
            close(sock);

            exit(5);
        }
        for (int i = 0; i < size; i++){
            text[i] = toupper(text[i]);
        }
        printf("%.*s", size, text);
        fflush(stdout);
    }
    printf("\n");
    clear(sock, &sock_struct);
    close(sock);

    exit(0);
}
