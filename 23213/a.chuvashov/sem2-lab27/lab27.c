#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#define BACKLOG 510

int working_flag = 1;

typedef struct {
    int client_fd;
    int remote_fd;
} Connection;

void handle_sigint(int sig) {
    working_flag = 0;
}

void handle_connection(int is_client, Connection* connection, struct pollfd* fds, int* nfds, int index) {
    char buffer[BUFSIZ];
    ssize_t dataRead;

    if (is_client) {
        dataRead = recv(connection->client_fd, buffer, BUFSIZ, 0);
        if (dataRead > 0) {
            send(connection->remote_fd, buffer, dataRead, 0);
        } else {
            close(connection->client_fd);
            close(connection->remote_fd);
            connection->client_fd = -1;
            connection->remote_fd = -1;

            for (int i = index; i < *nfds - 2; i++) fds[i] = fds[i + 2];
        }
    } 
    if(!is_client) {
        dataRead = recv(connection->remote_fd, buffer, BUFSIZ, 0);
        if (dataRead > 0) {
            send(connection->client_fd, buffer, dataRead, 0);
        } else {
            close(connection->client_fd);
            close(connection->remote_fd);
            connection->client_fd = -1;
            connection->remote_fd = -1;

            for (int i = index; i < *nfds - 2; i++) fds[i] = fds[i + 2];
        }
    }
}


int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "There should be 3 arguments: %s <listen_port> <remote_host> <remote_port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int listen_port;
    if ((listen_port = atoi(argv[1])) < 1) {
        fprintf(stderr, "Failed to parse listen port\n");
        return EXIT_FAILURE;
    }
    
    char *remote_host = argv[2];

    int remote_port;
    if ((remote_port = atoi(argv[3])) < 1) {
        fprintf(stderr, "Failed to parse remote port\n");
        return EXIT_FAILURE;
    }

    Connection conns[BACKLOG];

    for(int i = 0; i < BACKLOG; i++) {
        conns[i].client_fd = -1;
        conns[i].remote_fd = -1;
    }

    int listenfd;
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Can`t create socket");
        return EXIT_FAILURE;
    }

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt error");
        return EXIT_FAILURE;
    }

    struct sockaddr_in address_in;
    memset(&address_in, 0, sizeof(struct sockaddr_in));
    address_in.sin_family = AF_INET;
    address_in.sin_addr.s_addr = INADDR_ANY;
    address_in.sin_port = htons(listen_port);

    if (bind(listenfd, (struct sockaddr *)&address_in, sizeof(address_in)) < 0) {
        perror("Bind failure");
        return EXIT_FAILURE;
    }

    if (listen(listenfd, BACKLOG) == -1) {
        perror("Listen failure");
        return EXIT_FAILURE;
    }

    signal(SIGINT, handle_sigint);

    struct pollfd fds[BACKLOG * 2 + 1];
    memset(fds, 0, sizeof(fds));
    int nfds = 1;

    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    while (working_flag)
    {
        int ret = poll(fds, nfds, -1);
        if (ret == -1) {
            perror("Poll");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            int clientfd;
            if ((clientfd = accept(listenfd, NULL, NULL)) == -1) {
                perror("accept failure");
                continue;
            }

            struct hostent * host = gethostbyname(remote_host);
            if (host == NULL) {
                herror("gethostbyname");
                close(clientfd);
                continue;
            }
            
            int remotefd;
            if ((remotefd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Server socket failure");
                close(clientfd);
                continue;
            }

            struct sockaddr_in remote_in;
            memset(&remote_in, 0, sizeof(remote_in));
            remote_in.sin_family = AF_INET;
            remote_in.sin_port = htons(remote_port);
            remote_in.sin_addr = *(struct in_addr*)host->h_addr_list[0];

            if (connect(remotefd, (struct sockaddr*)&remote_in, sizeof(remote_in)) < 0) {
                perror("connect");
                close(clientfd);
                close(remotefd);
                continue;
            }

            if (nfds >= BACKLOG * 2 + 1) {
                fprintf(stderr, "Too many connections\n");
                close(remotefd);
                close(clientfd);
                continue;
            }

            conns[(nfds - 1) / 2].client_fd = clientfd;
            conns[(nfds - 1) / 2].remote_fd = remotefd;
            fds[nfds].fd = clientfd;
            fds[nfds].events = POLLIN;
            nfds++;

            fds[nfds].fd = remotefd;
            fds[nfds].events = POLLIN;
            nfds++; 
        }

        for(int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                Connection *con = &conns[(i-1)/2];
                int is_client = (fds[i].fd == con->client_fd);

                handle_connection(is_client, con, fds, &nfds, i);
            }
        }
        
    }
    return EXIT_SUCCESS;
}
