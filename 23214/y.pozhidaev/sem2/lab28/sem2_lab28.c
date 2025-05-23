#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>

int BIG_SIZE = 10000;
int SMALL_SIZE = 100;
int NUMBER_LINES = 25;

struct addrinfo hints, *res = NULL;
int sockfd = -1;

void parse_url(const char *url, char *host, char *path) {
    const char *protocol = "http://";
    if(strncmp(url, protocol, strlen(protocol)) == 0) {
        url += strlen(protocol);
    } else {
        fprintf(stderr, "Usage only: http://host/path\n");
        exit(1);
    }
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
    int header_end_found = 0;
    static char header_buffer[4096];
    int header_len = 0;

    char big_buffer[10000];
    int free_bytes = 10000;
    char buffer[100];
    int received;
    int count = 1;
    int code;
    int need_print = 1;
    int read_from = 0;
    int fill_from = 0;
    int end = 0;
    char host[BUFSIZ], path[BUFSIZ];
    char request[BUFSIZ*3];
    fd_set readfds;
    struct timeval timeout;

    if (argc < 2) {
        fprintf(stderr, "Usage only: http://host/path\n");
        exit(1);
    }
    parse_url(argv[1], host, path);

    signal(SIGINT, sigcatch);

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

    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, host);

    code = send(sockfd, request, strlen(request), 0);

    if(code == -1) {
        perror("send request");
        freeaddrinfo(res);
        close(sockfd);
        exit(1);
    }

    int max_fd = STDIN_FILENO > sockfd ? STDIN_FILENO : sockfd;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    set_non_canonical_mode();

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
        code = select(max_fd+1, &readfds, NULL, NULL, &timeout);
        if(code == -1){
            close(sockfd);
            freeaddrinfo(res);
            perror("Poll error");
            restore_canonical_mode();
            exit(1);
        }

        received = 0;
        if(FD_ISSET(sockfd, &readfds)) {
            if (free_bytes >= SMALL_SIZE) {
                if((received = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0){
                    if(received == -1){
                        freeaddrinfo(res);
                        close(sockfd);
                        perror("recv error");
                        restore_canonical_mode();
                        exit(1);
                    }
                    end = 1;
                } else {
                    int start = 0;

                    if (!header_end_found) {
                        for (int i = 0; i < received; i++) {
                            if (header_len < (int) sizeof(header_buffer) - 1)
                                header_buffer[header_len++] = buffer[i];

                            if (header_len >= 4 &&
                                strncmp(header_buffer + header_len - 4, "\r\n\r\n", 4) == 0) {
                                header_end_found = 1;
                                start = i + 1;
                                break;
                            }
                        }
                    } else {
                        start = 0;
                    }

                    if (header_end_found) {
                        for (int i = start; i < received; i++) {
                            big_buffer[fill_from] = buffer[i];
                            fill_from = (fill_from + 1) % (int) sizeof(big_buffer);
                            free_bytes--;
                        }
                    }
                }
            }
        }

        if(need_print) {
            if(end && free_bytes == BIG_SIZE) {
                printf("\nprocess finished\n");
                break;
            }
            char ch;
            int j = 0;
            for(; j < (int) sizeof (buffer) - 1 && free_bytes < BIG_SIZE; j++){
                ch = big_buffer[read_from];
                buffer[j] = ch;
                read_from = (read_from+ 1) % (int) sizeof(big_buffer);
                free_bytes++;
                if(ch == '\n') {
                    count++;
                    break;
                }
            }
            buffer[j+1] = '\0';
            printf("%s", buffer);
            fflush(stdout);
            if(count == NUMBER_LINES) {
                printf("\nSpace to continue\n");
                need_print = 0;
                count = 1;
            }
        } else {
            char ch;
            if(FD_ISSET(STDIN_FILENO, &readfds)) {
                read(STDIN_FILENO, &ch, 1);
                if(ch == ' '){
                    need_print = 1;
                }
            }
        }
    }

    restore_canonical_mode();
    close(sockfd);
    freeaddrinfo(res);
    exit(0);
}
