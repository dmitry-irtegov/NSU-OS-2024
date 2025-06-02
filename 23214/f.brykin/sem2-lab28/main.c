#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>
#include <errno.h>

#define CHUNK 256
#define PAGE 25
#define START_CAP 256
#define MAX_CAP (4 * 1024)

static struct termios oldt;

static void restore_term(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static void set_raw(void) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &oldt);
    t = oldt;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    atexit(restore_term);
}

static void clr_prompt(void) {
    write(STDOUT_FILENO, "\r\033[K", 4);
}

void split_url(char *u, char **proto, char **host, char **port_s, char **uri) {
    *proto = *host = *port_s = *uri = NULL;
    char *p = strstr(u, "://");
    if (p) {
        *p = 0;
        *proto = strdup(u);
        u = p + 3;
    } else {
        *proto = strdup("http");
    }
    p = strchr(u, '/');
    char *hp;
    if (p) {
        hp = strndup(u, p - u);
        *uri = strdup(p);
    } else {
        hp = strdup(u);
        *uri = strdup("/");
    }
    char *c = strchr(hp, ':');
    if (c) {
        *c = 0;
        *host = strdup(hp);
        *port_s = strdup(c + 1);
    } else {
        *host = strdup(hp);
    }
    free(hp);
}

int mk_sock(const char *h, int p) {
    struct hostent *he = gethostbyname(h);
    if (!he) return -1;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return -1;
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons(p);
    memcpy(&a.sin_addr.s_addr, he->h_addr, he->h_length);
    if (connect(s, (struct sockaddr *)&a, sizeof(a)) < 0) {
        close(s);
        return -1;
    }
    return s;
}

void send_req(int s, const char *h, const char *u) {
    char r[1024];
    snprintf(r, sizeof(r),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", u, h);
    send(s, r, strlen(r), 0);
}

void cleanup_resources(char *buf, int sockfd, char *proto, char *host, char *port_s, char *uri) {
    if (buf) free(buf);
    if (sockfd >= 0) close(sockfd);
    if (proto) free(proto);
    if (host) free(host);
    if (port_s) free(port_s);
    if (uri) free(uri);
}

int main(int ac, char **av) {
    if (ac != 2) return 1;

    char *pr = NULL, *ho = NULL, *po_s = NULL, *ur = NULL;
    split_url(av[1], &pr, &ho, &po_s, &ur);

    if (strcmp(pr, "http")) {
        cleanup_resources(NULL, -1, pr, ho, po_s, ur);
        return 1;
    }

    int port = po_s ? atoi(po_s) : 80;
    int sockfd = mk_sock(ho, port);
    if (sockfd < 0) {
        cleanup_resources(NULL, -1, pr, ho, po_s, ur);
        return 1;
    }

    send_req(sockfd, ho, ur);
    set_raw();

    size_t cap = START_CAP, len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        cleanup_resources(NULL, sockfd, pr, ho, po_s, ur);
        return 1;
    }

    enum { RUNNING, WAIT_PAGE, WAIT_BUF } state = RUNNING;

    int skip = 0, lines = 0, end = 0;
    char tmp[CHUNK];
    fd_set rf;

    while (1) {
        FD_ZERO(&rf);
        if (!end && state != WAIT_BUF) FD_SET(sockfd, &rf);
        if (state != RUNNING) FD_SET(STDIN_FILENO, &rf);

        select((sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO) + 1, &rf, NULL, NULL, NULL);

        if (!end && state != WAIT_BUF && FD_ISSET(sockfd, &rf)) {
            int n = recv(sockfd, tmp, sizeof(tmp), 0);
            if (n <= 0) {
                end = 1;
            } else {
                if (len + n >= cap) {
                    size_t new_cap = cap;
                    if (len + n >= new_cap) {
                        if ((new_cap * 2) <= MAX_CAP) {
                            new_cap *= 2;
                        }
                        else {
                            new_cap = MAX_CAP;
                        }
                    }
                    if (new_cap > cap) {
                        char *new_buf = realloc(buf, new_cap);
                        if (!new_buf) {
                            cleanup_resources(buf, sockfd, pr, ho, po_s, ur);
                            return 1;
                        }
                        buf = new_buf;
                        cap = new_cap;
                    } else {
                        state = WAIT_BUF;
                        fflush(stdout);
                        continue;
                    }
                }
                memcpy(buf + len, tmp, n);
                len += n;
            }
        }

        if (state == RUNNING && len > 0) {
            size_t pos = 0;
            if (!skip) {
                char *h = memmem(buf, len, "\r\n\r\n", 4);
                if (!h) {
                    memmove(buf, buf + len - 4, 4);
                    len = 4;
                    goto chk_end;
                }
                pos = (h - buf) + 4;
                skip = 1;
            }
            while (pos < len && lines < PAGE) {
                putchar(buf[pos]);
                if (buf[pos] == '\n') lines++;
                pos++;
            }
            if (pos > 0) {
                memmove(buf, buf + pos, len - pos);
                len -= pos;
            }
            if (lines >= PAGE) {
                printf("\n--- Press space to continue ---\n");
                fflush(stdout);
                state = WAIT_PAGE;
                lines = 0;
            }
        }

        if (state != RUNNING && FD_ISSET(STDIN_FILENO, &rf)) {
            char c;
            read(STDIN_FILENO, &c, 1);
            if (c == ' ') {
                clr_prompt();
                if (state == WAIT_BUF && len > 0) {
                    size_t pos = 0;
                    while (pos < len && lines < PAGE) {
                        putchar(buf[pos]);
                        if (buf[pos] == '\n') lines++;
                        pos++;
                    }
                    if (pos > 0) {
                        memmove(buf, buf + pos, len - pos);
                        len -= pos;
                    }
                    if (lines >= PAGE) {
                        printf("\n--- Press space to continue ---\n");
                        fflush(stdout);
                        state = WAIT_PAGE;
                        lines = 0;
                    } else {
                        state = RUNNING;
                    }
                } else {
                    state = RUNNING;
                }
            }
        }
chk_end:
        if (end && len == 0) break;
    }

    cleanup_resources(buf, sockfd, pr, ho, po_s, ur);
    return 0;
}

