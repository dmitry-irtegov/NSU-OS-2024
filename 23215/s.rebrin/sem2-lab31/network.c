#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "network.h"

void error(const char* msg);

void con_to_host(int* sockfd, char* host) {
    struct hostent* server;
    struct sockaddr_in serv_addr;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
        error("Ошибка открытия сокета");

    server = gethostbyname(host);
    if (server == NULL)
        error("Ошибка: нет такого хоста");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(80);

    if (connect(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Ошибка подключения");
}

void con_to_cli(int* sockfd) {
    struct sockaddr_in serv_addr;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
        error("Ошибка открытия сокета");

    int opt = 1;
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int rcvbuf = 16; // 256 байт
    setsockopt(*sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9002);

    if (bind(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Ошибка подключения");
}

void set_nonblocking(int cl_fd) {
    int flags = fcntl(cl_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL) failed");
        return;
    }

    if (fcntl(cl_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL) failed");
    }
}

void parse_http_request(const char* request, char* host) {
    char method[16], path[256], version[16];

    sscanf(request, "%15s %255s %15s", method, path, version);

    const char* host_header = strstr(request, "Host:");
    if (host_header) {
        sscanf(host_header, "Host: %255s", host);
        char* end = host + strlen(host) - 1;
        while (end >= host && (*end == '\r' || *end == '\n')) *end-- = '\0';
    }
}

int get_content_length_from_headers(const char* headers) {
    const char* content_length_header = "Content-Length:";
    const char* ptr = strstr(headers, content_length_header);

    if (ptr == NULL) {
        return -1;
    }

    ptr += strlen(content_length_header);
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }

    int length = 0;
    while (*ptr && isdigit(*ptr)) {
        length = length * 10 + (*ptr - '0');
        ptr++;
    }

    return length;
}