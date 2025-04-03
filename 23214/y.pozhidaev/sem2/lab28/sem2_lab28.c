#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include <poll.h>

struct addrinfo hints, *res;
int sockfd;

void parse_url(const char *url, char *host, char *path) {

    const char *slash = strchr(url, '/');
    if (slash) {
        strncpy(host, url, slash - url);
        host[slash - url] = '\0';
        strcpy(path, slash);
    } else {
        strcpy(host, url);
        strcpy(path, "/");
    }
}

void set_non_canonical_mode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void restore_canonical_mode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void sigcatch(){
    restore_canonical_mode();
    close(sockfd);
    freeaddrinfo(res);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage only http: %s host/path\n", argv[0]);
        exit(1);
    }

    char host[BUFSIZ], path[BUFSIZ];
    parse_url(argv[1], host, path);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, "80", &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        perror("socket");
        exit(1);
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        freeaddrinfo(res);
        close(sockfd);
        exit(1);
    }

    char request[BUFSIZ*3];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, host);

    send(sockfd, request, strlen(request), 0);

    char big_buffer[10001];
    int big_buf_cur_pos = 0;
    char buffer[101];
    int received;
    int count = 0;
    int code;
    int need_print = 1;


    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    set_non_canonical_mode();

    int end = 0;

    while (1) {
        code = poll(fds, 2, 5000);
        if(code == -1){
            close(sockfd);
            freeaddrinfo(res);
            perror("Poll error");
        }
        received = 0;
        if(fds[1].revents & POLLIN){
            if (need_print || big_buf_cur_pos <= (int) (sizeof(big_buffer) - sizeof(buffer))){
                if((received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) <= 0){
                    if(received == -1){
                        freeaddrinfo(res);
                        close(sockfd);
                        perror("recv error");
                    }
                    end = 1;
                }
            }
        }

        buffer[received] = '\0';

        if(need_print) {
            printf("%s", buffer);
            count++;
            if(end) {
                printf("\nprocess finished\n");
                break;
            }
            if(count == 25) {
                printf("\nSpace to continue\n");
                need_print = 0;
            }
        } else {
            strcpy(big_buffer+big_buf_cur_pos, buffer);
            big_buf_cur_pos += received;

            char ch;
            if(fds[0].revents & POLLIN) {
                read(STDIN_FILENO, &ch, 1);
                if(ch == ' '){
                    need_print = 1;
                    big_buffer[big_buf_cur_pos] = '\0';
                    printf("%s", big_buffer);
                    fflush(STDIN_FILENO);
                    big_buf_cur_pos = 0;
                    count = 0;
                    if(end) {
                        printf("\nprocess finished\n");
                        break;
                    }
                }
            }
        }
    }

    restore_canonical_mode();
    close(sockfd);
    freeaddrinfo(res);
    exit(0);
}
