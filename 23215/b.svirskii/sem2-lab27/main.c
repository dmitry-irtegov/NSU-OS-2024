#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define CONNECTIONS_LIMIT (510)

typedef struct Connection_t {
    int src_sock, dst_sock;
    struct Connection_t *next;
} Connection;

int listen_socket, dest_socket, nfds = 0;
fd_set read_fdset;
fd_set write_fdset;
Connection connections[CONNECTIONS_LIMIT];
int connections_count = 0;

void close_all() {
    close(listen_socket);
    close(dest_socket);
}

void add_nfds(int new_sock) {
    if (new_sock + 1 > nfds) {
        nfds = new_sock + 1;
    }
}

void init_listen_socket(int listen_port) {
    listen_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (listen_socket == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    int val = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &val,
                   sizeof(int)) == -1) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(struct sockaddr_in));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(listen_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listen_socket, (struct sockaddr *)&listen_addr,
             sizeof(listen_addr)) == -1) {
        perror("bind() listen socket failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_socket, CONNECTIONS_LIMIT) == -1) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    add_nfds(listen_socket);
    FD_SET(listen_socket, &read_fdset);
}

struct sockaddr_in dest_addr;

void set_dest_addr(char *dest_ip, int dest_port) {
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    switch (inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr)) {
    case 0:
        fprintf(stderr, "Invalid IPv4 address\n");
        exit(EXIT_FAILURE);
    case -1:
        perror("inet_pton() failed");
        exit(EXIT_FAILURE);
    }
}

Connection *empty_conn_list = NULL;
Connection *created_conn_list = NULL;

void create_connection() {
    if (empty_conn_list == NULL) {
        fprintf(stderr, "Too many connections!\n");
        return;
    }
    struct sockaddr_in new_conncetion_addr;
    unsigned int len = sizeof(struct sockaddr_in);
    Connection new_conn = {
        accept(listen_socket, (struct sockaddr *)&new_conncetion_addr, &len),
        socket(PF_INET, SOCK_STREAM, 0)};
    if (new_conn.src_sock == -1) {
        perror("accept() failed");
        exit(EXIT_FAILURE);
    }
    printf("accepted\n");

    if (new_conn.dst_sock == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    if (connect(new_conn.dst_sock, (struct sockaddr *)&dest_addr,
                sizeof(dest_addr)) == -1) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }
    printf("connected\n");

    Connection *new_conn_ptr = empty_conn_list;
    empty_conn_list = empty_conn_list->next;
    new_conn_ptr->next = created_conn_list;
    created_conn_list = new_conn_ptr;
    new_conn_ptr->dst_sock = new_conn.dst_sock;
    new_conn_ptr->src_sock = new_conn.src_sock;

    add_nfds(new_conn.dst_sock);
    add_nfds(new_conn.src_sock);
    FD_SET(new_conn.dst_sock, &read_fdset);
    FD_SET(new_conn.dst_sock, &write_fdset);
    FD_SET(new_conn.src_sock, &read_fdset);
    FD_SET(new_conn.src_sock, &write_fdset);
    connections_count++;
}

Connection *destroy_connection(Connection *conn) {
    if (created_conn_list == NULL) {
        fprintf(stderr, "No connections to destroy\n");
        exit(EXIT_FAILURE);
    }
    Connection *curr_conn = created_conn_list, *prev_conn = NULL;
    for (; curr_conn != NULL && curr_conn != conn;
         curr_conn = curr_conn->next) {
        prev_conn = curr_conn;
    };
    if (curr_conn == NULL) {
        fprintf(stderr, "Can't find connection to destroy\n");
        exit(EXIT_FAILURE);
    }
    close(conn->dst_sock);
    close(conn->src_sock);
    FD_CLR(conn->dst_sock, &read_fdset);
    FD_CLR(conn->dst_sock, &write_fdset);
    FD_CLR(conn->src_sock, &read_fdset);
    FD_CLR(conn->src_sock, &write_fdset);
    if (conn == created_conn_list) {
        created_conn_list = created_conn_list->next;
    } else {
        prev_conn->next = conn->next;
    }

    conn->next = empty_conn_list;
    empty_conn_list = conn;

    printf("destroyed\n");

    connections_count--;
    return conn->next;
}

int main(int argc, char **argv) {
    for (int i = 0; i < CONNECTIONS_LIMIT - 1; i++) {
        connections[i].next = &connections[i + 1];
    }
    empty_conn_list = &connections[0];
    FD_ZERO(&read_fdset);
    FD_ZERO(&write_fdset);
    if (argc < 4) {
        fprintf(stderr, "Invalid arguments count!\n");
        exit(EXIT_FAILURE);
    }

    uint16_t listen_port;
    sscanf(argv[1], "%hu", &listen_port);

    init_listen_socket(listen_port);

    uint16_t dest_port;
    sscanf(argv[3], "%hu", &dest_port);
    char *dest_ip = argv[2];

    set_dest_addr(dest_ip, dest_port);

    char buff[100];
    int bytes_count;
    while (1) {
        fd_set to_read = read_fdset;
        fd_set to_write = write_fdset;
        int code = select(nfds, &to_read, &to_write, NULL, NULL);
        if (code == -1) {
            perror("select() failed");
            exit(EXIT_FAILURE);
        } else if (code == 0) {
            continue;
        }

        if (FD_ISSET(listen_socket, &to_read)) {
            create_connection();
        }
        for (Connection *curr_conn = created_conn_list; curr_conn != NULL;) {
            int src_sock = curr_conn->src_sock, dst_sock = curr_conn->dst_sock;
            if (FD_ISSET(src_sock, &to_read) && FD_ISSET(dst_sock, &to_write)) {
                if ((bytes_count = recv(src_sock, buff, 100, 0)) == -1) {
                    perror("recv() failed");
                    exit(EXIT_FAILURE);
                } else if (bytes_count == 0) {
                    curr_conn = destroy_connection(curr_conn);
                    continue;
                }

                if ((bytes_count = send(dst_sock, buff, bytes_count, 0)) ==
                    -1) {
                    perror("send() failed");
                    exit(EXIT_FAILURE);
                } else if (bytes_count == 0) {
                    curr_conn = destroy_connection(curr_conn);
                    continue;
                }
            }

            if (FD_ISSET(dst_sock, &to_read) && FD_ISSET(src_sock, &to_write)) {
                if ((bytes_count = recv(dst_sock, buff, 100, 0)) == -1) {
                    perror("recv() failed");
                    exit(EXIT_FAILURE);
                } else if (bytes_count == 0) {
                    curr_conn = destroy_connection(curr_conn);
                    continue;
                }

                if ((bytes_count = send(src_sock, buff, bytes_count, 0)) ==
                    -1) {
                    perror("recv() failed");
                    exit(EXIT_FAILURE);
                } else if (bytes_count == 0) {
                    curr_conn = destroy_connection(curr_conn);
                    continue;
                }
            }
            curr_conn = curr_conn->next;
        }
    }

    exit(EXIT_SUCCESS);
}
