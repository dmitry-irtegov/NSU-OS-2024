#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <netdb.h>
#include <signal.h>

#define BUFF_SIZE 1024             
#define CONNECTIONS_LIMIT 510

typedef struct {
    int client_fd;
    int server_fd;
} Connection;

int server_running = 1;

void handle_sigint(int sig) {
    server_running = 0;
}

int create_server_socket(int port);
int connect_to_remote(const char* host, int port);
void handle_new_connection(int server_socket, const char* remote_host, int remote_port, 
    struct pollfd *fds, Connection *connections, int *num_fds);
void handle_client_data(Connection *con, struct pollfd *fds, int *num_fds, int index);
void handle_server_data(Connection *con, struct pollfd *fds, int *num_fds, int index);
void handle_data(int from, int to, Connection *con, struct pollfd *fds, int *num_fds, int index);
void close_connection(Connection *con, struct pollfd *fds, int *num_fds, int index);
void cleaning(int server_socket, struct pollfd *fds, int num_fds);

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <local_port> <remote_host> <remote_port>\n", argv[0]);
        return 1;
    }

    int local_port = atoi(argv[1]);
    const char* remote_host = argv[2];
    int remote_port = atoi(argv[3]);

    int server_socket = create_server_socket(local_port);
    if (server_socket == -1) {
        return 1;
    }

    printf("Listening on port %d and forwarding to %s: %d\n", 
           local_port, remote_host, remote_port);

    signal(SIGINT, handle_sigint);

    struct pollfd fds[CONNECTIONS_LIMIT * 2 + 1];
    Connection connections[CONNECTIONS_LIMIT] = {0};
    int num_fds = 1;

    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    while (server_running) {
        if (poll(fds, num_fds, -1) == -1) {
            if(errno == EINTR) {
                continue;
            }
            perror("poll error");
            break;
        }
        
        for (int i = 0; i < num_fds; i++) {
            if (fds[i].revents & POLLIN) {                
                if (i == 0) {
                    handle_new_connection(
                        server_socket, 
                        remote_host, 
                        remote_port, 
                        fds, 
                        connections, 
                        &num_fds);
                } else {
                    Connection *con = &connections[(i - 1) / 2];
                    if (fds[i].fd == con->client_fd) {
                        handle_client_data(connections, fds, &num_fds, i);
                    } else {
                        handle_server_data(connections, fds, &num_fds, i);
                    }
                }
            }
        }
    }

    cleaning(server_socket, fds, num_fds);
    return 0;
}

int create_server_socket(int port) {
    int socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_d == -1) {
        perror("socket error");
        return -1;
    }

    int opt = 1;
    if (setsockopt(socket_d, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt error");
        close(socket_d);
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(socket_d, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(socket_d);
        return -1;
    }

    if (listen(socket_d, CONNECTIONS_LIMIT) == -1) {
        perror("listen error");
        close(socket_d);
        return -1;
    }

    return socket_d;
}

int connect_to_remote(const char* host, int port) {
    struct hostent *remote_host = gethostbyname(host);
    if (remote_host == NULL) {
        herror("gethostbyname error");
        return -1;
    }

    int socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_d == -1) {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *(struct in_addr*)remote_host->h_addr_list[0];

    if ((connect(socket_d, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
        perror("connect error");
        close(socket_d);
        return -1;
    }

    return socket_d;
}

void handle_new_connection(int server_socket, const char* remote_host, int remote_port, 
                          struct pollfd *fds, Connection *connections, int *num_fds) {

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept error");
        return;
    }

    printf("New connection from %s: %d\n", 
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    int server_fd = connect_to_remote(remote_host, remote_port);
    if (server_fd == -1) {
        perror("Error connecting to remote server");
        close(client_fd);
        return;
    }

    printf("Connected to remote server %s: %d\n", remote_host, remote_port);

    if (*num_fds >= CONNECTIONS_LIMIT * 2 + 1) {
        fprintf(stderr, "Too many connections\n");
        close(client_fd);
        close(server_fd);
        return;
    }

    fds[*num_fds].fd = client_fd;
    fds[*num_fds].events = POLLIN;
    connections[(*num_fds - 1) / 2].client_fd = client_fd;

    fds[*num_fds + 1].fd = server_fd;
    fds[*num_fds + 1].events = POLLIN;
    connections[(*num_fds - 1) / 2].server_fd = server_fd;

    *num_fds += 2;
}

void handle_client_data(Connection *connections, struct pollfd *fds, int *num_fds, int index) {
    int conn_index = (index - 1) / 2;
    handle_data(connections[conn_index].client_fd, connections[conn_index].server_fd, 
                connections, fds, num_fds, index);

}

void handle_server_data(Connection *connections, struct pollfd *fds, int *num_fds, int index) {
    int conn_index = (index - 1) / 2;
    handle_data(connections[conn_index].server_fd, connections[conn_index].client_fd, 
                connections, fds, num_fds, index);
}

void handle_data(int from, int to, Connection *con, struct pollfd *fds, int *num_fds, int index) {
    char buffer[BUFF_SIZE];

    ssize_t bytes_read = recv(from, buffer, BUFF_SIZE, 0);

    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            fprintf(stderr, "Disconnected\n");
        } else {
            perror("recv error");
        }
        close_connection(con, fds, num_fds, index);
        return;
    }

    if ((send(to, buffer, bytes_read, 0)) <= 0) {
        perror("send error");
        close_connection(con, fds, num_fds, index);
    }
}

void close_connection(Connection *con, struct pollfd *fds, int *num_fds, int index) {
    printf("Closing connection: client_fd=%d, server_fd=%d\n", con->client_fd, con->server_fd);

    int con_id = (index-1)/2;

    close(con[con_id].client_fd);
    close(con[con_id].server_fd);

    con[con_id].client_fd = con[con_id].server_fd = -1;

    for (int i = con_id; i < (*num_fds - 1) / 2 - 1; i++) {
        con[i] = con[i + 1];
    }
    
    for (int i = index; i < *num_fds - 2; i++) {
        fds[i] = fds[i + 2];
    }

    *num_fds -= 2;
}

void cleaning(int server_socket, struct pollfd *fds, int num_fds) {

    close(server_socket);

    for (int i = 1; i < num_fds; i++) {
        if (fds[i].fd != -1) {
            close(fds[i].fd);
        }
    }
}
