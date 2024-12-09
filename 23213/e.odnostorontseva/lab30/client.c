#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    int client_d = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_d == -1) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    char* socket_path = "./socket";
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if ((connect(client_d, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
        perror("connect error");
        close(client_d);
        return 1;
    }

    printf("Connected to the server\n");
    printf("Enter a massage (Ctrl+D to exit):\n");
    char text[BUFSIZ];

    while (fgets(text, BUFSIZ, stdin) != NULL) {
        if (write(client_d, text, strlen(text)) == -1) {
            perror("write error");
            break;
        }
    }

    close(client_d);
    return 0;
}

