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
        error("Creating socket error");

    server = gethostbyname(host);
    if (server == NULL)
        error("Error: no such host");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(80);

    if (connect(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Connection error");
}

void con_to_cli(int* sockfd) {
    struct sockaddr_in serv_addr;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
        error("Opening socket error");

    int opt = 1;
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //int rcvbuf = 16; // 256 байт
    //setsockopt(*sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9002);

    if (bind(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Binding error");
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

void parse_headers(const char* headers, int* content_length, int* cache_live, int* status) {
    *content_length = -1;
    *cache_live = -1;
    *status = -1;

    const char* content_length_ptr = strstr(headers, "Content-Length:");
    if (content_length_ptr != NULL) {
        content_length_ptr += strlen("Content-Length:");
        while (*content_length_ptr && isspace(*content_length_ptr)) {
            content_length_ptr++;
        }

        *content_length = 0;
        while (*content_length_ptr && isdigit(*content_length_ptr)) {
            *content_length = *content_length * 10 + (*content_length_ptr - '0');
            content_length_ptr++;
        }
    }

    const char* cache_control_ptr = strstr(headers, "Cache-Control:");
    if (cache_control_ptr != NULL) {
        const char* max_age_ptr = strstr(cache_control_ptr, "max-age=");
        if (max_age_ptr != NULL) {
            max_age_ptr += strlen("max-age=");

            *cache_live = 0;
            while (*max_age_ptr && isdigit(*max_age_ptr)) {
                *cache_live = *cache_live * 10 + (*max_age_ptr - '0');
                max_age_ptr++;
            }
        }
    }

    const char* http_version_ptr = strstr(headers, "HTTP/");
    if (http_version_ptr != NULL) {
        // Пропускаем "HTTP/x.x "
        while (*http_version_ptr && !isspace(*http_version_ptr)) {
            http_version_ptr++;
        }
        while (*http_version_ptr && isspace(*http_version_ptr)) {
            http_version_ptr++;
        }

        *status = 0;
        while (*http_version_ptr && isdigit(*http_version_ptr)) {
            *status = *status * 10 + (*http_version_ptr - '0');
            http_version_ptr++;
        }
    }
}