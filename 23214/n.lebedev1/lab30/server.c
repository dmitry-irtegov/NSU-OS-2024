#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "socket.sock"

int listen_fd = -1;
int accepted_fd = -1;
int bind_flag = -1;

void int_sig_handler() {
    if (listen_fd != -1) {
        close(listen_fd);
    }
    if (accepted_fd != -1) {
        close(accepted_fd);
    }
    if (bind_flag == 0){
        unlink(SOCK_PATH);
    }
    exit(0);
}

int main() {
    signal(SIGHUP, int_sig_handler);
    signal(SIGINT, int_sig_handler);
    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Error with socket function");
        exit(-1);
    }
    printf("Created socket.\n");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    if ((bind_flag = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
        perror("Error with bind");
        close(listen_fd);
        exit(-1);
    }
    printf("Created socket end point with bind.\n");
    if (listen(listen_fd, 1) == -1) {
        perror("Error with listen");
        close(listen_fd);
        unlink(SOCK_PATH);
        exit(-1);
    }
    printf("Server socket is ready for listen.\n");
    if ((accepted_fd = accept(listen_fd, NULL, NULL)) == -1) {
        perror("Error with accept");
        close(listen_fd);
        unlink(SOCK_PATH);
        exit(-1);
    }
    printf("Connected to the client.\n");
    char msg[BUFSIZ];
    size_t length = sizeof(msg);
    ssize_t size;
    while ((size = read(accepted_fd, msg, length - 1)) > 0) {
        msg[size] = '\0';
        for (int i = 0; i < size; i++) {
            msg[i] = toupper((unsigned char)msg[i]);
        }
        msg[size] = '\0';
        printf("%s", msg);
    }
    if (size == -1) {
        perror("Error with read");
        close(accepted_fd);
        close(listen_fd);
        unlink(SOCK_PATH);
        exit(-1);
    }
    close(accepted_fd);
    close(listen_fd);
    unlink(SOCK_PATH);
    exit(0);
}
