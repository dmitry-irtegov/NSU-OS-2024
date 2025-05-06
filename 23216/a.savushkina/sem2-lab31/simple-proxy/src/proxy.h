#ifndef PROXY_H
#define PROXY_H

#define BUFFER_SIZE 8096
#define MAX_HEADER_SIZE 8192

int handle_client_request(int client_fd);

void process_request(int client_fd, const char *request, size_t req_len);

int set_nonblocking(int fd);

char *make_cache_key(const char *request);

#endif