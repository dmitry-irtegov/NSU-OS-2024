#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "socket"
#define MSG_SIZE 21

int main()
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[MSG_SIZE];

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("ERROR with socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("ERROR with bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1)
    {
        perror("ERROR with listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1)
    {
        perror("ERROR with accept");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(client_fd, buffer, MSG_SIZE)) > 0)
    {
        for (int i = 0; i < bytes_read; ++i)
        {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1)
        {
            perror("ERROR with write");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1)
    {
        perror("ERROR with read");
        close(server_fd);
        close(client_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return EXIT_SUCCESS;
}