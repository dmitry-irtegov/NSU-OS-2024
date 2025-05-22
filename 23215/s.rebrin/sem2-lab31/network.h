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
    int status_code;
    int working;
    data* dat;
    struct cache* next;
} cache;

typedef struct client {
    int cli_fd;              /* File descriptor for the client socket */
    int inet_fd;             /* File descriptor for the socket connected to the target host/server */
    char* host;              /* Pointer to a string containing the hostname of the target server */
    int tot;                 /* Total number of bytes read from the target server */
    int len;                 /* Expected length of the response body (content length) parsed from HTTP headers */
    int header_len;          /* Length of the HTTP headers in the response (up to \r\n\r\n) */
    int writing_to_client;   /* Number of bytes already written to the client socket in the current write operation */
    char buffer[4096];       /* Buffer to store data read from the target server or client */
    int writing;             /* Flag indicating whether the client is currently processing a request (1 - active, 0 - finished) */
    int writing_to_client_total; /* Total number of bytes to be written to the client socket in the current operation */
    cache* cur_cache;        /* Pointer to the current cache entry associated with the client's request */
    data* cur_data;          /* Pointer to the current data block from the cache (used when reading from cache) */
    time_t last_activity;    /* Timestamp of the last activity from the client (for timeout checking) */
    int using_cache;         /* Flag indicating whether the client is using cached data (1 - yes, 0 - no) */
    int caching;             /* Flag indicating whether the current response is being cached (1 - yes, 0 - no) */
    int headers_len;         /* Total length of the HTTP headers (including \r\n\r\n), used to determine the start of the body */
    int collect_headers;     /* Counter of bytes collected in headers_collectors, or -1 if header collection is complete */
    char* headers_collectors;/* Buffer for accumulating HTTP headers until fully parsed (e.g., until \r\n\r\n is found) */
    struct client* next;     /* Pointer to the next client in the linked list */
} client;

void con_to_host(int* sockfd, char* host);
void con_to_cli(int* sockfd);
void set_nonblocking(int cl_fd);
void parse_http_request(const char* request, char* host);
int get_content_length_from_headers(const char* headers);
void parse_headers(const char* headers, int* content_length, int* cache_live, int* status);

#endif