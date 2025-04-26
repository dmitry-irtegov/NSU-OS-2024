#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFSIZE 4096

typedef struct connection {
    int client_socket, forward_socket;
    struct connection *prev, *next;
    ssize_t data_count_client_to_forward, data_count_forward_to_client; 
    char buf_client_to_forward[BUFSIZE];
    char buf_forward_to_client[BUFSIZE];
} connection;

connection *head = NULL, *tail = NULL;
int nfds = 0;

void check_socket_against_nfds(int s) {
    if (s>=FD_SETSIZE) {
        fprintf(stderr, "Socket number out of range for select\n");
        exit(0);
    }
    if (s+1>nfds) nfds=s+1;
}

void drop_connection(connection *t) {
    if (t == head && t == tail) {
        head = tail =  NULL;
    } else if (t == head) {
        head = t->next;
        head->prev = NULL;
    } else if (t == tail) {
        tail = t->prev;
        tail->next = NULL;
    } else {
        assert(t->next != NULL);
        assert(t->prev != NULL);
        t->prev->next = t->next;
        t->next->prev = t->prev;
    }
    close(t->client_socket);
    close(t->forward_socket);
    free(t);
}

void form_select_fdsets(fd_set *readfds, fd_set *writefds, int serverfd) {
    FD_ZERO(readfds);
    FD_ZERO(writefds);
    FD_SET(serverfd, readfds);
    connection *t = head;
    while (t) {
        connection *t1;
        t1 = t->next;
        if ((t->data_count_client_to_forward < 0 && t->data_count_forward_to_client <= 0) 
        || (t->data_count_forward_to_client < 0 && t->data_count_client_to_forward <= 0)) {
            drop_connection(t);
        } else {
            if (t->data_count_client_to_forward == 0) {
                FD_SET(t->client_socket, readfds);
            }
            if (t->data_count_forward_to_client == 0) {
                FD_SET(t->forward_socket, readfds);
            }
            if (t->data_count_client_to_forward > 0) {
                FD_SET(t->forward_socket, writefds);
            }
            if (t->data_count_forward_to_client > 0) {
                FD_SET(t->client_socket, writefds);
            }
        }
        t = t1;
    }
}

void process_descriptors_and_pass_data(fd_set *readfds, fd_set *writefds) {
    connection *t = head;
    while(t) {
        if (t->data_count_client_to_forward == 0 && FD_ISSET(t->client_socket, readfds)) {
            t->data_count_client_to_forward = read(t->client_socket, t->buf_client_to_forward, sizeof(t->buf_client_to_forward));
            if (t->data_count_client_to_forward == 0) {
                t->data_count_client_to_forward = -1;
            }
        }
        if (t->data_count_forward_to_client == 0 && FD_ISSET(t->forward_socket, readfds)) {
            t->data_count_forward_to_client = read(t->forward_socket, t->buf_forward_to_client, sizeof(t->buf_forward_to_client));
            if (t->data_count_forward_to_client == 0) {
                t->data_count_forward_to_client = -1;
            }
        }
        if (t->data_count_client_to_forward > 0 && FD_ISSET(t->forward_socket, writefds)) {
            int res = write(t->forward_socket, t->buf_client_to_forward, t->data_count_client_to_forward);
            if (res == -1) {
                t->data_count_client_to_forward = -1;
            } else {
                t->data_count_client_to_forward = 0;
            }
        }
        if (t->data_count_forward_to_client > 0 && FD_ISSET(t->client_socket, writefds)) {
            int res = write(t->client_socket, t->buf_forward_to_client, t->data_count_forward_to_client);
            if (res == -1) {
                t->data_count_forward_to_client = -1;
            } else {
                t->data_count_forward_to_client = 0;
            }
        }
        t = t->next;
    }
}

connection* add_connection(int client_socket, struct sockaddr_in *serveraddr) {
    connection *t;
    t = malloc(sizeof(connection));
    t->client_socket = client_socket;
    t->forward_socket = socket(AF_INET, SOCK_STREAM, 0);
    check_socket_against_nfds(t->forward_socket);
    if (connect(t->forward_socket, (struct sockaddr*) serveraddr, sizeof(*serveraddr))) {
        perror("Connecting to server:");
        close(t->forward_socket);
        close(t->client_socket);
        free(t);
        return NULL;
    }
    t->prev = NULL;
    t->next = head;
    if (head == NULL) {
        tail = t;
    } else {
        head->prev = t;
    }
    head = t;
    t->data_count_forward_to_client = 0;
    t->data_count_client_to_forward = 0;
    return t;

}

int main(int argc, char* argv[]) {
    int server_port = atoi(argv[1]);
    int remote_port = atoi(argv[3]);
    if (server_port <= 0 || remote_port <= 0) {
        fprintf(stderr, "usage %s: server_port host remote_port", argv[0]);
        exit(0);
    }
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }
    if (server_socket == -1){
        perror("socket error");
        exit(1);
    }
    struct sockaddr_in server_addr, forward_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        perror(argv[0]);
        exit(1);
    }
    
    struct hostent* forward_host;
    forward_host = gethostbyname(argv[2]);
    if (forward_host == NULL) {
        fprintf(stderr, "getting host name error");
        exit(1);
    }
    memset(&forward_addr, 0, sizeof(forward_addr));
    forward_addr.sin_family = AF_INET;
    memcpy(&forward_addr.sin_addr.s_addr, forward_host->h_addr_list[0], sizeof(struct in_addr));
    forward_addr.sin_port = htons(remote_port);

    signal(SIGPIPE, SIG_IGN);

    fd_set readfds, writefds;
    int nready = 0;
    int addrlen;
    listen(server_socket, 510);
    check_socket_against_nfds(server_socket);
    while(1) {
        form_select_fdsets(&readfds, &writefds, server_socket);
        nready = select(nfds, &readfds, &writefds, NULL, NULL);
        if (nready == -1) {
            perror(argv[0]);
            break;
        }
        process_descriptors_and_pass_data(&readfds, &writefds);
        
        if (FD_ISSET(server_socket, &readfds)) {
            int client_connection;
            addrlen = sizeof(client_addr);
            client_connection = accept(server_socket, (struct sockaddr*) &client_addr, &addrlen);
            if (client_connection < 0) {
                perror(argv[0]);
                break;
            }
            check_socket_against_nfds(client_connection);
            if (!add_connection(client_connection, &forward_addr)) {
                perror(argv[0]);
                break;
            }
        }
    }
    exit(0);
}
