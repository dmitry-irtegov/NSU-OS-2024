#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef LEN_BUF
#define LEN_BUF 10
#endif

int initSocket(char* socketname) {
    struct sockaddr_un addr;

    int soc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (soc == -1){
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketname, sizeof(addr.sun_path)-1);

    if (bind(soc, (struct sockaddr*)&addr, sizeof(addr))) {
        return -1;
    }

    if (listen(soc, 1) == -1) {
        return -1;
    }

    return soc;
}

int workWithConnection(int soc){
    char sym;
    char buf[LEN_BUF];
    int len;

    int new = accept(soc, NULL, NULL);
    if (new == -1) {
        return -1;
    }

    puts("Getting started");
    while ((len = read(new, buf, LEN_BUF)) != 0) {
        for (int i = 0; i < len; i++) {
            sym = toupper(buf[i]);
            if (write(1, &sym, 1) == -1){
                close(new);
                return -1;
            }
        }
    }

    return 0;
}

void closeAndUnlick(int soc, char* nameSocket){
    close(soc);
    unlink(nameSocket);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fputs("missed socket name", stderr);
        exit(EXIT_FAILURE);
    }

    int soc = initSocket(argv[1]);
    if (soc == -1) {
        perror("initSocket failed");
        exit(EXIT_FAILURE);
    }

    if (workWithConnection(soc)){
        perror("initSocket failed");
        closeAndUnlick(soc, argv[1]);
        exit(EXIT_FAILURE);
    }
    

    closeAndUnlick(soc, argv[1]);
    exit(EXIT_SUCCESS);
}