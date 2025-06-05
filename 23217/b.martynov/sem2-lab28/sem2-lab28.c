#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define CNT_LINES 25

char CLEANER[] = "\t\t\t\t\r";

struct termios old;

void returnTTY() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &old) != 0) {
        perror("terminal error");
        exit(EXIT_FAILURE);
    } 
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // setup terminal
    struct termios term;
    if(tcgetattr(STDIN_FILENO, &term) != 0) {
        perror("tcgetattr error");
        exit(1);
    }   
    old = term;
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;

    atexit(returnTTY);

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != 0) {
        perror("tcsetattr error");
        exit(1);
    }   


    // разбить на домен и ресурс
    char url[BUFSIZ];
    if (strlcpy(url, argv[1], BUFSIZ) >= BUFSIZ) {
        fprintf(stderr, "url is longer than BUFSIZ\n");
    }

    char* token = strtok(url, "/");
    if (token == NULL) {
        fprintf(stderr, "Invalid URL (don't use empty string and only '/')\n");
        exit(EXIT_SUCCESS);
    }

    // now *token is "http:\0\0az.lib.ru/\0"

    if (strcmp(token, "http:") != 0) {
        fprintf(stderr, "Use http:// at the beginning of the URL.\n");
        exit(EXIT_FAILURE);
    }

    if ((token = strtok(NULL, "/")) == NULL) {
        fprintf(stderr, "Use part after http:// in the URL (for example az.lib.ru).\n");
        exit(EXIT_FAILURE);
    }
    
    // now *token is "az.lib.ru\0\0"

    char host[BUFSIZ] = { 0 };
    if (strlcpy(host, token, BUFSIZ) >= BUFSIZ) {
        fprintf(stderr, "host is longer than BUFSIZ\n");
    }

    char res[BUFSIZ] = { 0 };
    while ((token = strtok(NULL, "/")) != NULL) {
        strcat(res, "/");
        strcat(res, token);
    }

    size_t len_path = strlen(res);
    if (len_path == 0) {
        res[0] = '/';
        res[1] = 0;
    }

    char request[4 * BUFSIZ] = { 0 };
    snprintf(request, 4 * BUFSIZ, 
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        res, host
    );



    struct hostent* host_addr = gethostbyname(host);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, host_addr->h_addr_list[0], host_addr->h_length);
    addr.sin_port = htons(80);

    int socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_d == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_d, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    ssize_t sended;
    size_t len_request = strlen(request);
    if ((sended = send(socket_d, request, len_request, 0)) == -1) {
        perror("send error");
        exit(EXIT_FAILURE);
    }
    if(sended != len_request) {
        fprintf(stderr, "Not all data was sended\n");
        exit(EXIT_FAILURE);
    }



    fd_set fds;
    size_t offset = 0;
    int count_lines = 0;
    int until = 0;

    char buf[128 * BUFSIZ];
    while (1) {
        FD_ZERO(&fds);
        FD_SET(socket_d, &fds);
        FD_SET(STDIN_FILENO, &fds);

        if (select(socket_d + 1, &fds, NULL, NULL, NULL) == -1) {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            char symbol;
            ssize_t cnt;
            if ((cnt = read(STDIN_FILENO, &symbol, 1)) == 1) {
                if (symbol == ' ' && count_lines >= CNT_LINES) {
                    count_lines = 0;
                    printf(CLEANER);
                    fflush(stdout);
                }
            }
            if (cnt == -1) {
                perror("read stdin error");
                exit(EXIT_FAILURE);
            }
        }

        size_t readed;
        if (FD_ISSET(socket_d, &fds)) {
            if ((readed = read(socket_d, buf + offset, (sizeof(buf) - 1) - offset)) == -1) {
                perror("read socket error");
                exit(EXIT_FAILURE);
            }

            if (readed == 0 && offset == 0) {
                break;
            }

            if (readed > 0) {
                offset += readed;
                buf[offset] = '\0';
            }

            until = 0;
            if (offset > 0 && count_lines < CNT_LINES) {
                for (int i = 0; i < offset; i++) {
                    if (buf[i] == '\n') {
                        until = i + 1;
                        count_lines++;
                        if (count_lines >= CNT_LINES) {
                            break;
                        }
                    }
                }

                if (count_lines < CNT_LINES) {
                    write(STDOUT_FILENO, buf, offset);
                    offset = 0;
                    buf[0] = '\0';
                }
                // >= CNT_LINES
                // вывести всё, что можно. Т.е. до CNT_LINES строк
                else if (until > 0) {
                    write(STDOUT_FILENO, buf, until);

                    // for (int i = until; i < sizeof(buf); i++) {
                    //     buf[i - until] = buf[i];
                    // }


                    memmove(buf, buf + until, offset - until);

                    offset -= until;
                }

                if (count_lines >= CNT_LINES) {
                    printf("Press Space to scroll down\r");
                    fflush(stdout);
                }
            }
        }
    }

    if (close(socket_d) == -1) {
        perror("close error");
    }
    exit(EXIT_SUCCESS);
}
