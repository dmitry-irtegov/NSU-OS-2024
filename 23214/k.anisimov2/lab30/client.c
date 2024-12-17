#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "socket"

int main()
{
    int client_fd;
    char *text = "Message to convert.\n";
    struct sockaddr_un addr;

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("ERROR with socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("ERROR with connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (write(client_fd, text, strlen(text)) == -1)
    {
        perror("ERROR with write");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    close(client_fd);
    return EXIT_SUCCESS;
}