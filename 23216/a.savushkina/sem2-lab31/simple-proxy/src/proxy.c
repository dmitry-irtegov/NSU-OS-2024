#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "proxy.h"
#include "cache.h"
#include "utils.h"

int handle_client_request(int client_fd);
void process_request(int client_fd, char *request);
char *extract_host(char *request);
int should_cache_response(char *request);
int set_nonblocking(int fd);

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get failed");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set failed");
        return -1;
    }
    return 0;
}

int handle_client_request(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    char incorrect_method[44] = "Incorrect http method. Use only get or head\n";

    while (1) {
        bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            if (strcmp(buffer, "CLOSE_CONNECTION") == 0) {
                printf("Received close connection command\n");
                return 0;
            }
            if (!should_keep_connection(buffer)) {
                write(client_fd, incorrect_method, strlen(incorrect_method));
                return 0;
            }
            process_request(client_fd, buffer);
        } else if (bytes_read == 0) {
            printf("Client disconnected\n");
            break;
        } else if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                perror("read error");
                break;
            }
        }
    }

    return 0;
}

void process_request(int client_fd, char *request) {
    const char *cached_response = get_from_cache(request);
    printf("Request: %s\n", request);
    int cache_age = should_cache_response(request);
    if (cached_response && cache_age > 0) {
        write(client_fd, cached_response, strlen(cached_response));
        printf("Response from cache\n");
        return;
    }
    printf("Response from server\n");
    char *host = extract_host(request);
    printf("Host: %s\n", host);
    if (!host) {
        fprintf(stderr, "Failed to extract host from request\n");
        return;
    }
    printf("befor\n");
    struct hostent *remote_host = gethostbyname(host);
    printf("after\n");
    if (!remote_host || !remote_host->h_addr_list || !remote_host->h_addr_list[0]) {
        perror("gethostbyname");
        printf("Failed to resolve host: %s\n", host);
        free(host);
        return;
    }
    printf("Host: %s\n", remote_host->h_name);

    int remote_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_fd < 0) {
        perror("socket");
        free(host);
        return;
    }

    printf("Connecting to remote server\n");

    struct sockaddr_in remote_address;
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(80);
    memcpy(&remote_address.sin_addr, remote_host->h_addr_list[0], remote_host->h_length);

    if (connect(remote_fd, (struct sockaddr *)&remote_address, sizeof(remote_address)) < 0) {
        perror("connect");
        close(remote_fd);
        free(host);
        return;
    }

    printf("Connected to remote server\n");

    if (send(remote_fd, request, strlen(request), 0) < 0) {
        perror("send failed");
        close(remote_fd);
        free(host);
        return;
    }
    printf("Request sent to remote server\n");

    char *response = malloc(BUFFER_SIZE);
    if (!response) {
        perror("malloc failed");
        close(remote_fd);
        free(host);
        return;
    }
    printf("Sending request to remote server\n");

    ssize_t response_length;
    ssize_t total_bytes_received = 0;
    ssize_t content_length = content_length_provided(response);
    printf("Content-Length: %zd\n", content_length);
    if (content_length > 0) {
        printf("Content-Length provided: %zd\n", content_length);
    } else {
        printf("Content-Length not provided\n");
    }
    if (cache_age > 0) {
        printf("Cache age: %d\n", cache_age);
    } else {
        printf("No cache age provided\n");
    }

    while ((response_length = recv(remote_fd, response, BUFFER_SIZE - 1, 0)) > 0) {
        printf("Received %zd bytes from remote server\n", response_length);
        response[response_length] = '\0';
        total_bytes_received += response_length;

        if (content_length > 0) {
            double percentage = ((double)total_bytes_received / content_length) * 100;
            printf("Downloaded: %.2f%%\n", percentage);
        }

        if (!content_length_provided(response)) {
            printf("Content-Length not provided. Switching to pass-through mode.\n");
            cache_age = 0;
        }
        if (cache_age && !enough_memory_for_cache(response_length)) {
            printf("Not enough memory for caching. Switching to pass-through mode.\n");
            cache_age = 0;
        }
        if (cache_age) {
            add_to_cache(request, response, cache_age);
        }
        write(client_fd, response, response_length);
    }

    if (response_length == 0) {
        mark_cache_entry_complete(request);
    }

    if (response_length < 0) {
        perror("recv error");
    }

    free(response);
    close(remote_fd);
    free(host);
}