#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>

#define PAGE_LINES 25
#define MAX_LINE 1024
#define BUFFER_CAPACITY 65536
#define DEFAULT_PORT 80

struct termios original_termios;
int socket_fd = -1;
struct addrinfo *resolved_info = NULL;

typedef struct
{
    char *content;
    size_t length;
} TextLine;

typedef struct
{
    TextLine entries[BUFFER_CAPACITY];
    size_t start;
    size_t end;
    size_t count;
    size_t read_pos;
} LineQueue;

LineQueue line_queue = {.start = 0, .end = 0, .count = 0, .read_pos = 0};

void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

void set_raw_terminal()
{
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &original_termios) != 0)
        exit(EXIT_FAILURE);
    raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void handle_exit(int sig)
{
    (void)sig;
    restore_terminal();
    if (socket_fd >= 0)
        close(socket_fd);
    if (resolved_info)
        freeaddrinfo(resolved_info);
    exit(EXIT_SUCCESS);
}

void add_line(const char *text)
{
    while (line_queue.count == BUFFER_CAPACITY)
    {
        printf("Buffer full. Press SPACE to continue...\n");
        fflush(stdout);
        char c;
        while (read(STDIN_FILENO, &c, 1) > 0)
        {
            if (c == ' ')
                break;
        }

        int shown = 0;
        while (shown < PAGE_LINES && line_queue.read_pos != line_queue.end)
        {
            const char *line = line_queue.entries[line_queue.read_pos].content;
            if (line)
            {
                printf("%s\n", line);
                free((void *)line);
                line_queue.entries[line_queue.read_pos].content = NULL;
                line_queue.read_pos = (line_queue.read_pos + 1) % BUFFER_CAPACITY;
                line_queue.count--;
                shown++;
            }
        }
    }

    line_queue.entries[line_queue.end].content = strdup(text);
    line_queue.entries[line_queue.end].length = strlen(text);
    line_queue.end = (line_queue.end + 1) % BUFFER_CAPACITY;
    line_queue.count++;
}

const char *next_line()
{
    if (line_queue.read_pos == line_queue.end)
        return NULL;
    const char *result = line_queue.entries[line_queue.read_pos].content;
    line_queue.entries[line_queue.read_pos].content = NULL;
    line_queue.read_pos = (line_queue.read_pos + 1) % BUFFER_CAPACITY;
    line_queue.count--;
    return result;
}

void parse_url(const char *url, char *host, size_t host_size, char *path, size_t path_size, int *port)
{
    *port = DEFAULT_PORT;
    if (strncmp(url, "http://", 7) == 0)
        url += 7;

    const char *slash = strchr(url, '/');
    const char *colon = strchr(url, ':');

    if (colon && (!slash || colon < slash))
    {
        snprintf(host, host_size, "%.*s", (int)(colon - url), url);
        *port = atoi(colon + 1);
    }
    else if (slash)
    {
        snprintf(host, host_size, "%.*s", (int)(slash - url), url);
    }
    else
    {
        snprintf(host, host_size, "%s", url);
        snprintf(path, path_size, "/");
        return;
    }

    if (slash)
        snprintf(path, path_size, "%s", slash);
    else
        snprintf(path, path_size, "/");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_exit);
    atexit(restore_terminal);
    set_raw_terminal();

    char hostname[1024], path[2048];
    int port;
    parse_url(argv[1], hostname, sizeof(hostname), path, sizeof(path), &port);

    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char port_string[16];
    snprintf(port_string, sizeof(port_string), "%d", port);

    if (getaddrinfo(hostname, port_string, &hints, &resolved_info) != 0)
    {
        fprintf(stderr, "ERROR getaddrinfo: %s\n", gai_strerror(errno));
        exit(EXIT_FAILURE);
    }

    socket_fd = socket(resolved_info->ai_family, resolved_info->ai_socktype, resolved_info->ai_protocol);
    if (socket_fd < 0)
    {
        fprintf(stderr, "ERROR socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, resolved_info->ai_addr, resolved_info->ai_addrlen) < 0)
    {
        fprintf(stderr, "ERROR connect: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char http_request[4096];
    snprintf(http_request, sizeof(http_request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, hostname);

    if (send(socket_fd, http_request, strlen(http_request), 0) < 0)
    {
        fprintf(stderr, "ERROR send: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buffer[4096], line_buf[MAX_LINE];
    int line_len = 0, skip_headers = 0, shown = 0;

    fd_set fds;

    while (1)
    {
        FD_ZERO(&fds);
        FD_SET(socket_fd, &fds);
        FD_SET(STDIN_FILENO, &fds);

        int max_fd = socket_fd > STDIN_FILENO ? socket_fd : STDIN_FILENO;
        int ready = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if (ready < 0)
        {
            fprintf(stderr, "ERROR select: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(socket_fd, &fds))
        {
            int bytes = recv(socket_fd, buffer, sizeof(buffer), 0);
            if (bytes <= 0)
                break;

            for (int i = 0; i < bytes; ++i)
            {
                char ch = buffer[i];
                if (ch == '\r')
                    continue;
                if (ch == '\n')
                {
                    line_buf[line_len] = '\0';
                    if (!skip_headers)
                    {
                        if (line_len == 0)
                            skip_headers = 1;
                    }
                    else
                    {
                        add_line(line_buf);
                    }
                    line_len = 0;
                }
                else
                {
                    if (line_len < MAX_LINE - 1)
                    {
                        line_buf[line_len++] = ch;
                    }
                }
            }
        }

        while (shown < PAGE_LINES && line_queue.read_pos != line_queue.end)
        {
            const char *line = next_line();
            if (line)
            {
                printf("%s\n", line);
                free((void *)line);
                shown++;
            }
        }

        if (shown == PAGE_LINES)
        {
            printf("Press SPACE to continue...\n");
            fflush(stdout);
            char c;
            while (read(STDIN_FILENO, &c, 1) > 0)
            {
                if (c == ' ')
                    break;
            }
            shown = 0;
        }
    }

    while (line_queue.read_pos != line_queue.end)
    {
        const char *line = next_line();
        if (line)
        {
            printf("%s\n", line);
            free((void *)line);
        }
    }

    restore_terminal();
    if (resolved_info)
        freeaddrinfo(resolved_info);
    if (socket_fd >= 0)
        close(socket_fd);
    exit(EXIT_SUCCESS);
}
