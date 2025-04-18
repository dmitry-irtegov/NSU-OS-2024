#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define SCREEN_LINES 25

struct termios old;

void returnTTY() {
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH, &old) != 0) {
        perror("terminal");
        exit(1);
    } 
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        exit(1);
    }

    struct termios term;
    if(tcgetattr(STDIN_FILENO, &term) != 0) {
        perror("terminal");
        exit(1);
    }   
    old = term;
    term.c_lflag &= ~(ICANON | ECHO);

    atexit(returnTTY);

    if(tcsetattr(STDIN_FILENO,TCSAFLUSH, &term) != 0) {
        perror("terminal");
        exit(1);
    }   
    

    char buf[BUFSIZ];
    char *url = argv[1];
    // char url[] = "http://az.lib.ru/";

    char *token = strtok(url, "/");
    if (token == NULL) {
        fprintf(stderr, "Invalid URL\n");
        exit(1);
    }

    if (strcmp(token, "http:") == 0) {
        token = strtok(NULL, "/");
    } else if(strcmp(token, "http:") != 0) {
        fprintf(stderr, "Invalid adress. Use http\n");
        exit(1);
        
    }

    char host[BUFSIZ/3];
    strcpy(host, token);
    char path[BUFSIZ/3] = "/";
    while ((token = strtok(NULL, "/")) != NULL) {
        strcat(path, token);
        strcat(path, "/");
    }
    size_t lenPath = strlen(path);
    if(lenPath > 1) {
        path[lenPath-1] = '\0';
    }

    // Формируем HTTP запрос
    char request[BUFSIZ];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n", path, host);
    printf("%s\n\n", request);

    struct hostent *host_addr = gethostbyname(host);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, host_addr->h_addr_list[0], host_addr->h_length);
    addr.sin_port = htons(80);

    int dsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dsocket == -1) {
        perror("socket");
        exit(1);
    }

    if (connect(dsocket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }

    ssize_t sended;
    size_t lenRequest = strlen(request);
    if ((sended = send(dsocket, request, lenRequest, 0)) == -1) {
        perror("send");
        exit(1);
    }
    if(sended != lenRequest) {
        fprintf(stderr, "Not all data was sended\n");
        exit(1);
    }

    fd_set fds;
    int offset = 0;
    int lines_count = 0;
    int prev = 0;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(dsocket, &fds);
        FD_SET(STDIN_FILENO, &fds);

        if (select(dsocket + 1, &fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        if(FD_ISSET(STDIN_FILENO, &fds)) {
            char s;
            if (read(STDIN_FILENO, &s, 1) == 1) {
                if (lines_count == SCREEN_LINES) {
                    if (s == ' ') {
                        lines_count = 0;
                    }
                }
            }
        }

        if (FD_ISSET(dsocket, &fds)) {
            if(lines_count == SCREEN_LINES && offset == sizeof(buf)) {
                continue;
            }
            int ridden;
            if ((ridden = read(dsocket, buf + offset, sizeof(buf) - offset)) == -1) {
                perror("read");
                exit(1);
            }
            if(ridden == 0 && offset == 0) {
                break;
            }

            offset += ridden;
            prev = 0;
            int flag = 0;
            if(lines_count == SCREEN_LINES) {
                continue;
            }

            for (int i = 0; i < offset; i++) {
                if (buf[i] == '\n') {
                    write(STDOUT_FILENO, buf + prev, i - prev + 1);
                    prev = i + 1;
                    lines_count++;
                    flag = 1;
                    if(lines_count == SCREEN_LINES) {
                        printf("Press space to scroll down ");
                        fflush(stdout);
                        break;
                    }
                }
            }
            if (flag == 0) {
                write(STDOUT_FILENO, buf, offset);
            }
            

            if (prev != 0) {
                memmove(buf, buf + prev, offset - prev);
                offset -= prev;
            } else {
                offset = 0;
            }
        }

    }
    
    close(dsocket);
    return 0;
}
