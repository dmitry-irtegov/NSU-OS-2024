#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <time.h>

#define BUFFER_SIZE 4096

typedef struct data {
    int len;
    char* data;
    struct data* next;
} data;

typedef struct cache {
    char* request;
    int live_time;
    time_t birth_time;
    int working;
    data* dat;
    struct cache* next;
} cache;

typedef struct client {
    int cli_fd;
    int inet_fd;
    char* host;
    int tot;
    int len;
    int header_len; 
    int writing_to_client;
    char buffer[4096];
    int writing;
    int writing_to_client_total;
    cache* cur_cache;
    data* cur_data;
    time_t last_activity;
    int using_cache;
    int headers_len;
    struct client* next;
} client;

void con_to_host(int* sockfd, char* host);
void con_to_cli(int* sockfd);
void set_nonblocking(int cl_fd);
void parse_http_request(const char* request, char* host);
int get_content_length_from_headers(const char* headers);

#endif