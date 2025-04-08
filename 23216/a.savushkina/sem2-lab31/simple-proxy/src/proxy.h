#ifndef PROXY_H
#define PROXY_H

#define BUFFER_SIZE 4096

int handle_client_request(int client_fd);

void process_request(int client_fd, char *request);

int set_nonblocking(int fd);

#endif