#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#ifndef LEN_BUF
#define LEN_BUF 10
#endif

int connectToSocket(char* socketname) {
    struct sockaddr_un addr;

    int soc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (soc == -1){
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketname, sizeof(addr.sun_path)-1);

    if (connect(soc, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        return -1;
    }

    return soc;
}

void sigpipeHandler(){
    write(STDERR_FILENO, "Lost connection with server\n", strlen("Lost connection with server\n") + 1);
    _exit(EXIT_FAILURE);
}

int workWithConnection(char* socketname){
    char buf[LEN_BUF];
    int len;
    int soc = connectToSocket(socketname);
    if (soc == -1) {
        return -1;
    }

    signal(SIGPIPE, sigpipeHandler);
    puts("For end of work please enter CTRL-D");
    while (1) {
        len = read(STDIN_FILENO, buf, LEN_BUF);
        if (len == 0) {
            break;
        }
        else if (len < 0) {
            close(soc);
            return -1;
        }

        if (write(soc, buf, len) <= 0) {
            close(soc);
            return -1;
        }
    }

    close(soc);
    return 0;
}

int main(int argc, char** argv){
    if (argc < 2) {
        fputs("missed socket name\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (workWithConnection(argv[1])) {
        perror("workWithConnection failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}