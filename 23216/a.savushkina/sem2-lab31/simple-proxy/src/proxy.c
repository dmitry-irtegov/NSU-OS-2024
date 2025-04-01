#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "proxy.h"
#include "cache.h"
#include "utils.h"

int handle_client_request(int client_fd);
void process_request(int client_fd, char *request);
char *extract_host(char *request);
int should_cache_response(char *request);

int handle_client_request(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    char incorrect_method[44] = "Incorrect http method. Use only get or head\n";

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        if (strcmp(buffer, "CLOSE_CONNECTION") == 0) {
            printf("Received close connection command\n");
            return 0;
        }
        if (!should_keep_connection(buffer)){
            write(client_fd, incorrect_method, strlen(incorrect_method));
            return 0;
        }
        process_request(client_fd, buffer);
    }

    if (bytes_read == 0) {
        printf("Client disconnected\n");
    } else if (bytes_read < 0) {
        perror("read error");
    }

    return 0;
}

void process_request(int client_fd, char *request) {
    const char *cached_response = get_from_cache(request);
    printf("Request: %s\n", request);
    if (cached_response && should_cache_response(request)) {
        write(client_fd, cached_response, strlen(cached_response));
        printf("Response from cache\n");
    } else {
        printf("Response from server\n");
        char *host = extract_host(request);
        if (!host) {
            fprintf(stderr, "Failed to extract host from request\n");
            return;
        }

        struct hostent *remote_host = gethostbyname(host);
        if (!remote_host) {
            perror("gethostbyname");
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
        memcpy(&remote_address.sin_addr, remote_host->h_addr, remote_host->h_length);

        if (connect(remote_fd, (struct sockaddr *)&remote_address, sizeof(remote_address)) < 0) {
            perror("connect");
            close(remote_fd);
            free(host);
            return;
        }

        printf("Connected to remote server\n");

        send(remote_fd, request, strlen(request), 0);
        char response[BUFFER_SIZE];
        ssize_t response_length;
        while ((response_length = recv(remote_fd, response, sizeof(response) - 1, 0)) > 0) {
            response[response_length] = '\0';
            if (should_cache_response(request)) {
                add_to_cache(request, response);
            }
            write(client_fd, response, response_length);
        }

        if (response_length < 0) {
            perror("recv error");
        }
        close(remote_fd);
        free(host);
    }
}