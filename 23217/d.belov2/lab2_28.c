#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>

#define BUFSIZE 4096
#define SCREEN_LINES 25

typedef struct {
    int socket;
    struct termios original_term;
    char buffer[BUFSIZE];
    int buffer_pos;
    int lines_displayed;
    int waiting_for_space;
    int connection_closed;
} StateManager;

StateManager* global_state = NULL;

void restore_terminal() {
    if (global_state) {
        tcsetattr(STDIN_FILENO, TCSANOW, &global_state->original_term);
    }
}

void handle_signal(int sig) {
    restore_terminal();
    exit(1);
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);   
    sigaction(SIGTERM, &sa, NULL);  
}

void setup_terminal(StateManager* state) {
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &state->original_term);
    new_term = state->original_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    new_term.c_cc[VMIN] = 1;
    new_term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void parse_url(const char *url, char *host, char *path) {
    if (strncmp(url, "http://", 7) != 0) {
        fprintf(stderr, "URL must start with http://\n");
        exit(1);
    }

    const char *start = url + 7;
    const char *slash = strchr(start, '/');
    if (slash) {
        strncpy(host, start, slash - start);
        host[slash - start] = '\0';
        strcpy(path, slash);
    } else {
        strcpy(host, start);
        strcpy(path, "/");
    }
}

int connect_to_host(const char *host) {
    struct hostent *he = gethostbyname(host);
    if (!he) {
        perror("gethostbyname");
        exit(1);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);
    addr.sin_port = htons(80);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    return sock;
}

void send_request(int sock, const char *host, const char *path) {
    char req[BUFSIZE];
    snprintf(req, sizeof(req),
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n\r\n",
        path, host);
    send(sock, req, strlen(req), 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s http://host/path\n", argv[0]);
        return 1;
    }

    StateManager state = {0};
    global_state = &state;
    
    setup_signal_handlers();
    atexit(restore_terminal);

    char host[256], path[1024];
    parse_url(argv[1], host, path);

    setup_terminal(&state);
    state.socket = connect_to_host(host);
    send_request(state.socket, host, path);

    char buf[BUFSIZE];
    int offset = 0;
    int lines = 0;
    int wait_space = 0;
    fd_set fds;

    while (1) {
        FD_ZERO(&fds);
        if (!wait_space) FD_SET(state.socket, &fds);
        FD_SET(STDIN_FILENO, &fds);

        if (select(state.socket + 1, &fds, NULL, NULL, NULL) < 0) {
            perror("select");
            restore_terminal();
            close(state.socket);
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            char c;
            read(STDIN_FILENO, &c, 1);
            if (wait_space && c == ' ') {
                lines = 0;
                wait_space = 0;
                printf("\r\033[K"); 
                fflush(stdout);
            }
        }

        if (FD_ISSET(state.socket, &fds) && !wait_space) {
            int n = read(state.socket, buf + offset, BUFSIZE - offset);
            if (n <= 0 && offset == 0) break; 

            offset += n;

            int start = 0;
            for (int i = 0; i < offset; ++i) {
                if (buf[i] == '\n') {
                    write(STDOUT_FILENO, buf + start, i - start + 1);
                    start = i + 1;
                    lines++;
                    if (lines >= SCREEN_LINES) {
                        write(STDOUT_FILENO, "Press space to continue\n", 25);
                        wait_space = 1;
                        break;
                    }
                }
            }

            if (!wait_space && start < offset) {
                write(STDOUT_FILENO, buf + start, offset - start);
                start = offset;
            }

            if (start < offset) {
                memmove(buf, buf + start, offset - start);
                offset -= start;
            } else {
                offset = 0;
            }
        }
    }

    restore_terminal();
    close(state.socket);
    return 0;
}
