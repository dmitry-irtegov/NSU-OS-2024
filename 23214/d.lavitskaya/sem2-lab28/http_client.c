#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <termios.h>

#define BUF_SIZE 4096
#define MAXLINES 25

void parse_url(char *url, char **protocol, char **user, char **hostname,
              char **port, char **uri) {
    *protocol = NULL;
    *user = NULL;
    *hostname = NULL;
    *port = NULL;
    *uri = NULL;

    char *p = strstr(url, "://");  
    if (p) {
       *p = '\0';
       *protocol = strdup(url);
       url = p + 3; 
    }

    p = strchr(url, '@');
    if (p) {
        *p = '\0';
        *user = strdup(url);
        url = p + 1;
    }

    p = strchr(url, '/');
    char *hostport;
    if (p) {
        hostport = strndup(url, p - url);
        *uri = strdup(p);
    } else {
        hostport = strdup(url);
        *uri = strdup("/");
    }

    char *colon = strchr(hostport, ':');
    if (colon) {
        *colon = '\0';
        *hostname = strdup(hostport);
        *port = strdup(colon + 1);
    } else {
        *hostname = strdup(hostport);
    }

    free(hostport);
}

int open_socket(char *hostname, int port) {
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Host not found: %s\n", hostname);
        close(sockfd);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void send_get_request(int sockfd, const char *host, const char *uri) {
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             uri, host);
    send(sockfd, request, strlen(request), 0);   
}

void set_noncanonical_mode(struct termios *orig) {
    struct termios newt;
    tcgetattr(STDIN_FILENO, orig);
    newt = *orig;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN]  = 1;
    newt.c_cc[VTIME] = 0; 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restore_terminal(struct termios *orig) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }

    char *protocol, *user, *hostname, *port_string, *uri;
    int port;

    parse_url(argv[1], &protocol, &user, &hostname, &port_string, &uri);
    
    if (protocol == NULL || strcmp(protocol, "http") != 0) {
        fprintf(stderr, "Only HTTP protocol is supported\n");
        return 1;
    }

    if (user != NULL) {
        fprintf(stderr, "User credentials in URL are not supported\n");
        return 1;
    }

    port = (port_string != NULL) ? atoi(port_string) : 80;
    if (port <= 0) {
        fprintf(stderr, "Invalid port number\n");
        return 1;
    }

    int sockfd = open_socket(hostname, port);
    if(sockfd < 0) {
        return 1;
    }

    send_get_request(sockfd, hostname, uri);

    struct termios orig_term;
    set_noncanonical_mode(&orig_term);

    int header_passed = 0;
    int header_state  = 0;
    int lines = 0;
    int lowmark = 0, highmark = 0;
    int empty = 1, full = 0, eof = 0;
    int paused = 0;
    char buf[BUF_SIZE];
    fd_set readfs, writefs;

    while (1) {
        FD_ZERO(&readfs);
        FD_ZERO(&writefs);

        if (!full && !eof) {
            FD_SET(sockfd, &readfs);
        } 

        if (!empty && lines < MAXLINES && !paused) {
            FD_SET(1, &writefs);
        }

        if (paused) {
            FD_SET(0, &readfs);
        }
            

        int res = select(sockfd + 1, &readfs, &writefs, NULL, NULL);
        if (res < 0) {
            perror("select");
            break;
        }

        if (!full && !eof && FD_ISSET(sockfd, &readfs)) {
            if (!header_passed) {
                char c;
                while (!header_passed) {
                    res = read(sockfd, &c, 1);
                    if (res <= 0) {
                        eof = 1; 
                        break; 
                    }
                    switch (header_state) {
                        case 0: header_state = (c == '\r') ? 1 : 0; break;
                        case 1: header_state = (c == '\n') ? 2 : (c=='\r'?1:0); break;
                        case 2: header_state = (c == '\r') ? 3 : (c=='\r'?1:0); break;
                        case 3:
                            if (c == '\n') {
                                header_passed = 1;
                            } else {
                                header_state = (c=='\r'?1:0);
                            }
                            break;
                    }
                }
            } else {
                int free_bufer_space = (highmark >= lowmark)
                        ? BUF_SIZE - highmark
                        : lowmark - highmark;
    
                res = read(sockfd, buf + highmark, free_bufer_space);
                if (res <= 0) {
                    eof = 1;
                } else {
                    highmark = (highmark + res) % BUF_SIZE;
                    empty = 0;
                    if (highmark == lowmark) full = 1;
                }
            }
        }

        if (!empty && lines < MAXLINES && FD_ISSET(1, &writefs) && !paused) {
            char *start = buf + lowmark;
            char *end = (lowmark < highmark) ? buf + highmark : buf + BUF_SIZE;
            char *p = start;
    
            int line = 0;
            while (p < end && *p != '\n') p++;
            if (p < end) {
                p++; 
                line = 1;
            }

            int len = p - start;
            res = write(1, start, len);
            if (res > 0) {
                lowmark = (lowmark + res) % BUF_SIZE;
                lines += line;
                if (lowmark == highmark) {
                    empty = 1;
                }
                full = 0;
    
                if (lines >= MAXLINES) {
                    write(1, "\n--- Press space to scroll down ---\n\n", 37);
                    paused = 1;
                    tcflush(STDIN_FILENO, TCIFLUSH);
                }
            }
        }
    
        if (paused && FD_ISSET(0, &readfs)) {
            char input_buf[BUF_SIZE];
            int n = read(0, input_buf, BUF_SIZE);
            if (n > 0) {
                int i = 0;
                for (i; i < n; ++i) {
                    if (input_buf[i] == ' ') {
                        lines = 0;
                        paused = 0;
                        break;
                    }
                }
            }
        }
    
        if (eof && empty) break;
    }

    restore_terminal(&orig_term);
    close(sockfd);
    return 0;

}
