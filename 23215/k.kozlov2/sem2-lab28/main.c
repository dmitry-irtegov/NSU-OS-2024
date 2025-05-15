#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <termios.h>

#define PORT 80

void parse_url(char *url, char *host, char *path) {
    if (strncmp(url, "http://", 7) != 0) {
        fprintf(stderr, "bad url\n");
        exit(EXIT_FAILURE);
    }

    url += 7;

    if (*url == 0) {
        fprintf(stderr, "no host\n");
        exit(EXIT_FAILURE);
    }

    strcpy(host, url);

    char *sl = strchr(url, '/');

    if (sl) {
        host[sl - url] = 0;
        strcpy(path, sl);
    } else {
        strcpy(path, "/");
    }
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "usage: %s <url>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *url = argv[1];
    char path[1024];
    char host[1024];

    parse_url(url, host, path);

    struct hostent *serv = gethostbyname(host);
    if (serv == NULL) {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *(struct in_addr *)serv->h_addr_list[0];

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char request[4096];
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    fd_set read_fds;
    char buffer[BUFSIZ];
    char stopped = 0;
    char has_more = 0;
    char *line = NULL;
    int lines = 0;
    int not_first = 0;

    struct termios orig_term, new_term;
    tcgetattr(0, &orig_term);
    new_term = orig_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_term);

    while (1) {

        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(0, &read_fds);

        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            close(sockfd);
            tcsetattr(0, TCSANOW, &orig_term);
            exit(EXIT_FAILURE);
        }

        if (!stopped && FD_ISSET(sockfd, &read_fds)) {

            if (!has_more) {
                ssize_t bytes_received = recv(sockfd, buffer, BUFSIZ - 1, 0);
                if (bytes_received < 0) {
                    perror("recv");
                    close(sockfd);
                    tcsetattr(0, TCSANOW, &orig_term);
                    exit(EXIT_FAILURE);
                } else if (bytes_received == 0) {
                    break;
                }
                buffer[bytes_received] = '\0';
                line = strtok(buffer, "\n");
            } else {
                line = strtok(NULL, "\n");
            }

            while (line != NULL) {
                if (lines == 0 && not_first) {
                    printf("\r%-40s\n", line);
                } else {
                    printf("%s\n", line);
                }
                lines++;

                if (lines == 25) {
                    lines = 0;
                    stopped = 1;
                    has_more = 1;
                    not_first = 1;
                    printf("----- Press space to scroll down -----");
                    break;
                }

                line = strtok(NULL, "\n");
            }

            fflush(stdout);
            
            if (line == NULL) {
                has_more = 0;
            }
        }

        if (stopped && FD_ISSET(0, &read_fds)) {
            if (getchar() == ' ') {
                stopped = 0;
            }
        }
    }

    close(sockfd);
    tcsetattr(0, TCSANOW, &orig_term);
    exit(EXIT_SUCCESS);
}
