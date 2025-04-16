#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>


#define MAX_CON 510
#define MAX_BUFFER 1024

struct pollfd connections[2 * MAX_CON+1];
int num_connections = 1;

struct socket_pair {
    int client;
    int server;
    char buffer_client[MAX_BUFFER];
    int buffer_client_size;
    char buffer_server[MAX_BUFFER];
    int buffer_server_size;
};

struct socket_pair * pairs[2 * MAX_CON];

int listen_port;
int remote_port;
struct hostent * remote_host;

int proxy_socket;

void handler(int signum) {
    exit(EXIT_SUCCESS);
}

void create_proxy_socket() {
    proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket < 0) {
        perror("Can't create proxy socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(listen_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxy_socket, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(proxy_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    connections[0].fd = proxy_socket;
    connections[0].events = POLLIN;
    connections[0].revents = 0;

}

void create_connection() {
    int client_fd = accept(proxy_socket, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        close(client_fd);
        return;
    }

    struct sockaddr_in remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    remote_addr.sin_addr = *(struct in_addr *)remote_host->h_addr_list[0];
    if (connect(server_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
        perror("connect");
        close(server_fd);
        close(client_fd);
        return;
    }
    
    struct socket_pair * pair = malloc(sizeof(struct socket_pair));
    if (pair == NULL) {
        perror("malloc");
        close(client_fd);
        close(server_fd);
        return;
    }

    pair->client = client_fd;
    pair->server = server_fd;
    pair->buffer_client_size = 0;
    pair->buffer_server_size = 0;
    memset(pair->buffer_client, 0, MAX_BUFFER);
    memset(pair->buffer_server, 0, MAX_BUFFER);
    
    pairs[client_fd] = pair;
    pairs[server_fd] = pair;
    connections[num_connections].fd = client_fd;
    connections[num_connections].events = POLLIN;
    connections[num_connections].revents = 0;
    num_connections++;
    connections[num_connections].fd = server_fd;
    connections[num_connections].events = POLLIN;
    connections[num_connections].revents = 0;
    num_connections++;
}

void close_connection(struct socket_pair * pair, int closed_fd, int closed_fd_connection_index) {
    int peer_fd = (pair->client == closed_fd) ? pair->server : pair->client;
    struct pollfd peer_pfd = (closed_fd_connection_index % 2 == 1) ? 
        connections[closed_fd_connection_index + 1] : connections[closed_fd_connection_index - 1];
    char * buffer = (pair->client == closed_fd) ? pair->buffer_client : pair->buffer_server;
    int buffer_size = (pair->client == closed_fd) ? pair->buffer_client_size : pair->buffer_server_size;

    int ret = poll(&peer_pfd, 1, 0);

    if (ret > 0 && (peer_pfd.revents & POLLOUT)) {
        write(peer_fd, buffer, buffer_size);
    }

    free(pair);

    int connections_index = (closed_fd_connection_index % 2 == 1) ? closed_fd_connection_index : closed_fd_connection_index - 1;
    for (int i = connections_index; i < num_connections - 2; i++) {
        connections[i] = connections[i + 2];
    }
    num_connections -= 2;


    close(peer_fd);
    close(closed_fd);

}

void run_proxy() {

    while (1) {
        poll(connections, num_connections, -1);
        for(int i = 1; i < num_connections; i++) {
            if (connections[i].revents & POLLIN ) {
                connections[i].revents &= ~POLLIN;
                struct socket_pair * pair = pairs[connections[i].fd];

                char * buffer;
                int buffer_available_size;
                int * buffer_size;
                int connection_peer_index = (i%2==1) ? i+1 : i-1;
                if (connections[i].fd == pair->client) {
                    buffer = pair->buffer_client;
                    buffer_available_size = MAX_BUFFER - pair->buffer_client_size;
                    buffer_size = &pair->buffer_client_size;
                } else {
                    buffer = pair->buffer_server;
                    buffer_available_size = MAX_BUFFER - pair->buffer_server_size;
                    buffer_size = &pair->buffer_server_size;
                }

                if(buffer_available_size == 0) {
                    continue;
                }

                char local_buffer[MAX_BUFFER];
                memset(local_buffer, 0, MAX_BUFFER);
                int bytes_read = read(connections[i].fd, local_buffer, buffer_available_size);


                if (bytes_read <= 0) {
                    close_connection(pair, connections[i].fd, i);
                    continue;
                }
                connections[connection_peer_index].events |= POLLOUT;
                memcpy(buffer + *buffer_size, local_buffer, bytes_read);
                *buffer_size += bytes_read;
            }
            if (connections[i].revents & POLLOUT) {
                connections[i].revents &= ~POLLOUT;
                struct socket_pair * pair = pairs[connections[i].fd];

                char * buffer;
                int buffer_available_size;
                int * buffer_size;
                if (connections[i].fd == pair->server) {
                    buffer = pair->buffer_client;
                    buffer_available_size = MAX_BUFFER - pair->buffer_client_size;
                    buffer_size = &pair->buffer_client_size;
                } else {
                    buffer = pair->buffer_server;
                    buffer_available_size = MAX_BUFFER - pair->buffer_server_size;
                    buffer_size = &pair->buffer_server_size;
                }



                if (*buffer_size > 0) {
                    int bytes_written = write(connections[i].fd, buffer, *buffer_size);
                    
                    if (bytes_written < 0) {
                        perror("write");
                        close_connection(pair, connections[i].fd, i);
                        continue;
                    }
                    *buffer_size -= bytes_written;
                    memmove(buffer, buffer + bytes_written, *buffer_size);

                    if(*buffer_size == 0){
                        connections[i].events &= ~POLLOUT;
                    }
                }
            }
        }
        if (connections[0].revents & POLLIN) {
            connections[0].revents &= ~POLLIN;
            if (num_connections >= 2 * MAX_CON) {
                fprintf(stderr, "Max connections reached\n");
                continue;
            }
            
            create_connection();
        }
    }
}

int main (int argc, char ** argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <P> <NAME OR IP> <P'>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if((listen_port = atoi(argv[1])) <= 0) {
        fprintf(stderr, "Invalid port number: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if((remote_host = gethostbyname(argv[2])) == NULL) {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    if((remote_port = atoi(argv[3])) <= 0) {
        fprintf(stderr, "Invalid port number: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handler);
    create_proxy_socket();
    run_proxy();

    exit(EXIT_SUCCESS);
}
