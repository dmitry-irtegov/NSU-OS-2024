#include <iostream>
#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <string>
#include <cstring>
#include <unordered_map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>

#define BACKLOG 510
using namespace std;

vector<struct pollfd> fds;
unordered_map<int, int> conns, rev_conns;
int listenfd;
int listen_port;
int remote_port;
string remote_host;
int working_flag = 1;

void handle_sigint(int sig) {
    working_flag = 0;
}

void close_connection(int fd_index) {
    int fd = fds[fd_index].fd;
    int otherfd;
    if (conns.count(fd)) {
        otherfd = conns[fd];
        conns.erase(fd);
        rev_conns.erase(otherfd);
    } else if (rev_conns.count(fd)) {
        otherfd = rev_conns[fd];
        rev_conns.erase(fd);
        conns.erase(otherfd);
    }
    
    fds.erase(fds.begin() + fd_index);
    close(fd);
    
    for(size_t i = 1; i < fds.size(); i++) {
        if (fds[i].fd == otherfd)
        {
            fds.erase(fds.begin() + i);
            close(otherfd);
            break;
        }
    }

}

void handle_connection(int index) {
    char buffer[BUFSIZ];
    ssize_t dataRead;

    int from = fds[index].fd;
    int to = conns.count(from) ? conns[from] : rev_conns[from];
    
    if ((dataRead = read(from, buffer, BUFSIZ)) > 0) {
        write(to, buffer, dataRead);
    } else if (dataRead == 0){
        close_connection(index);
    } else if (dataRead == -1) {
        perror("read");
        close_connection(index);
    }
}

int remote_connect() {
    struct hostent * host = gethostbyname(remote_host.data());
    if (host == NULL) {
        herror("gethostbyname");
        return -1;
    }

    int remotefd;
    if ((remotefd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Server socket failure");
        return -1;
    }

    struct sockaddr_in remote_in;
    memset(&remote_in, 0, sizeof(remote_in));
    remote_in.sin_family = AF_INET;
    remote_in.sin_port = htons(remote_port);
    remote_in.sin_addr = *(struct in_addr*)host->h_addr_list[0];

    if (connect(remotefd, (struct sockaddr*)&remote_in, sizeof(remote_in)) < 0) {
        perror("connect");
        close(remotefd);
        return -1;
    }
    return remotefd;
}


void proxy_server() {
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Can`t create socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address_in;
    memset(&address_in, 0, sizeof(struct sockaddr_in));
    address_in.sin_family = AF_INET;
    address_in.sin_addr.s_addr = INADDR_ANY;
    address_in.sin_port = htons(listen_port);

    if (bind(listenfd, (struct sockaddr *)&address_in, sizeof(address_in)) < 0) {
        perror("Bind failure");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, BACKLOG) == -1) {
        perror("Listen failure");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);
    
    fds.push_back({listenfd, POLLIN, 0});
    
    while (working_flag) {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret == -1) {
            perror("Poll");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            int clientfd;
            if ((clientfd = accept(listenfd, nullptr, nullptr)) == -1) {
                perror("accept failure");
                continue;
            }
            
            int remotefd = remote_connect();
            if (remotefd == -1) {
                close(clientfd);
                continue;
            } 

            if (fds.size() >= BACKLOG * 2 + 1) {
                fprintf(stderr, "Too many connections\n");
                close(remotefd);
                close(clientfd);
                continue;
            }

            conns[clientfd] = remotefd; 
            rev_conns[remotefd] = clientfd;
            fds.push_back({clientfd, POLLIN, 0});
            fds.push_back({remotefd, POLLIN, 0});
        }

        for(size_t i = 1; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) 
                handle_connection(i);
        }   
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        cerr << "There should be 3 arguments: " << argv[0] << " <listen_port> <remote_host> <remote_port>\n";
        return EXIT_FAILURE;
    }

    if ((listen_port = atoi(argv[1])) < 1) {
        cerr << "Failed to parse listen port\n";
        return EXIT_FAILURE;
    }
    
    remote_host = argv[2];

    if ((remote_port = atoi(argv[3])) < 1) {
        cerr << "Failed to parse remote port\n";
        return EXIT_FAILURE;
    }

    proxy_server();
    
    return EXIT_SUCCESS;
}
