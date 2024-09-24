#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <termios.h>
#include <sys/select.h>
#include <signal.h>

#define RING_BUFFER_SIZE 4096
#define TEMP_BUFFER_SIZE 1024
#define LEFTOVER_SIZE TEMP_BUFFER_SIZE
#define LINES_PER_PAGE 25
#define DEFAULT_HTTP_PORT "80"
#define HTTP_PROTOCOL_PREFIX "://"
#define DEFAULT_HTTP_PATH "/"

struct termios orig_terminal_settings;

static unsigned char leftover[LEFTOVER_SIZE];
static size_t leftover_len = 0;
static size_t leftover_off = 0;

void feed_ring_from(unsigned char *ring_buffer,
                    int *write_position,
                    int read_position,
                    const unsigned char *src,
                    size_t *src_len,
                    size_t *src_off)
{
    while (*src_off < *src_len)
    {
        int next_wp = (*write_position + 1) % RING_BUFFER_SIZE;
        if (next_wp == read_position)
        {
            break;
        }
        ring_buffer[*write_position] = src[*src_off];
        *write_position = next_wp;
        (*src_off)++;
    }
    if (*src_off >= *src_len)
    {
        *src_off = *src_len = 0;
    }
}

void parse_url(const char *url_input,
               char **parsed_host,
               char **parsed_port,
               char **parsed_path)
{
    const char *current_ptr = url_input;
    *parsed_port = strdup(DEFAULT_HTTP_PORT);
    *parsed_path = strdup(DEFAULT_HTTP_PATH);

    const char *protocol_sep = strstr(current_ptr, HTTP_PROTOCOL_PREFIX);
    if (protocol_sep)
    {
        current_ptr = protocol_sep + strlen(HTTP_PROTOCOL_PREFIX);
    }

    const char *path_sep = strchr(current_ptr, '/');
    size_t host_port_length = path_sep ? (size_t)(path_sep - current_ptr) : strlen(current_ptr);
    char *host_port = strndup(current_ptr, host_port_length);

    if (path_sep)
    {
        free(*parsed_path);
        *parsed_path = strdup(path_sep);
    }

    char *port_sep = strchr(host_port, ':');
    if (port_sep)
    {
        *port_sep = '\0';
        free(*parsed_port);
        *parsed_port = strdup(port_sep + 1);
    }
    *parsed_host = strdup(host_port);
    free(host_port);

    if ((*parsed_host)[0] == '\0')
    {
        fprintf(stderr, "ERROR parse_url: Invalid hostname.\n");
        exit(EXIT_FAILURE);
    }
}

int establish_connection(const char *hostname, int port_number)
{
    struct hostent *host_entry = gethostbyname(hostname);
    if (!host_entry)
    {
        fprintf(stderr, "ERROR establish_connection: %s\n", hostname);
        return -1;
    }

    int connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_socket < 0)
    {
        fprintf(stderr, "ERROR socket: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    memcpy(&server_address.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    if (connect(connection_socket,
                (struct sockaddr *)&server_address,
                sizeof(server_address)))
    {
        fprintf(stderr, "ERROR connection: %s\n", strerror(errno));
        close(connection_socket);
        return -1;
    }
    return connection_socket;
}

void configure_raw_terminal()
{
    struct termios new_terminal_settings;
    if (tcgetattr(STDIN_FILENO, &orig_terminal_settings) < 0)
    {
        fprintf(stderr, "ERROR tcgetattr: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    new_terminal_settings = orig_terminal_settings;
    new_terminal_settings.c_lflag &= ~(ICANON | ECHO);
    new_terminal_settings.c_cc[VMIN] = 1;
    new_terminal_settings.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_settings) < 0)
    {
        fprintf(stderr, "ERROR tcsetattr: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void restore_terminal()
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &orig_terminal_settings) < 0)
    {
        fprintf(stderr, "ERROR tcsetattr: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void skip_http_headers(int socket_fd)
{
    char parsing_state = 0;
    char current_char;
    while (read(socket_fd, &current_char, 1) == 1)
    {
        switch (parsing_state)
        {
        case 0:
            parsing_state = (current_char == '\r') ? 1 : 0;
            break;
        case 1:
            parsing_state = (current_char == '\n') ? 2 : 0;
            break;
        case 2:
            parsing_state = (current_char == '\r') ? 3 : 0;
            break;
        case 3:
            if (current_char == '\n')
                return;
            parsing_state = 0;
            break;
        }
    }
}

void handle_signal(int signal)
{
    (void)signal;
    restore_terminal();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_signal);
    atexit(restore_terminal);

    char *hostname = NULL, *port_str = NULL, *path = NULL;
    parse_url(argv[1], &hostname, &port_str, &path);

    int port_number = atoi(port_str);
    if (port_number <= 0)
    {
        fprintf(stderr, "ERROR invalid port: %s\n", port_str);
        exit(EXIT_FAILURE);
    }

    int connection_socket = establish_connection(hostname, port_number);
    if (connection_socket < 0)
    {
        exit(EXIT_FAILURE);
    }

    char http_request[512];
    snprintf(http_request, sizeof(http_request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, hostname);
    if (write(connection_socket,
              http_request,
              strlen(http_request)) < 0)
    {
        fprintf(stderr, "ERROR write: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    skip_http_headers(connection_socket);
    configure_raw_terminal();

    unsigned char ring_buffer[RING_BUFFER_SIZE];
    int read_pos = 0;
    int write_pos = 0;
    int displayed_lines = 0;
    int is_paused = 0;
    int connection_closed = 0;

    while (1)
    {
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        int buffer_not_full = ((write_pos + 1) % RING_BUFFER_SIZE != read_pos);
        int buffer_not_empty = (read_pos != write_pos);

        if (!connection_closed && buffer_not_full)
        {
            FD_SET(connection_socket, &read_fds);
        }
        if (!is_paused && buffer_not_empty)
        {
            FD_SET(STDOUT_FILENO, &write_fds);
        }
        if (is_paused)
        {
            FD_SET(STDIN_FILENO, &read_fds);
        }

        if (select(connection_socket + 1, &read_fds, &write_fds, NULL, NULL) < 0)
        {
            fprintf(stderr, "ERROR select: %s\n", strerror(errno));
            break;
        }

        if (!connection_closed && buffer_not_full)
        {
            if (leftover_len > 0)
            {
                feed_ring_from(ring_buffer, &write_pos, read_pos, leftover, &leftover_len, &leftover_off);
            }
            if (FD_ISSET(connection_socket, &read_fds) && leftover_len == 0)
            {
                unsigned char temp_buffer[TEMP_BUFFER_SIZE];
                ssize_t bytes_read = read(connection_socket, temp_buffer, sizeof(temp_buffer));
                if (bytes_read <= 0)
                {
                    connection_closed = 1;
                }
                else
                {
                    size_t temp_len = (size_t)bytes_read;
                    size_t temp_off = 0;
                    feed_ring_from(ring_buffer, &write_pos, read_pos, temp_buffer, &temp_len, &temp_off);
                    if (temp_off < temp_len)
                    {
                        size_t remain = temp_len - temp_off;
                        if (remain > LEFTOVER_SIZE)
                        {
                            remain = LEFTOVER_SIZE;
                        }
                        memcpy(leftover, temp_buffer + temp_off, remain);
                        leftover_len = remain;
                        leftover_off = 0;
                    }
                }
            }
        }

        if (FD_ISSET(STDOUT_FILENO, &write_fds))
        {
            unsigned char output_byte = ring_buffer[read_pos];
            if (write(STDOUT_FILENO, &output_byte, 1) < 0)
            {
                fprintf(stderr, "ERROR write: %s\n", strerror(errno));
                break;
            }
            read_pos = (read_pos + 1) % RING_BUFFER_SIZE;
            if (output_byte == '\n' && ++displayed_lines >= LINES_PER_PAGE)
            {
                is_paused = 1;
                displayed_lines = 0;
                const char *pause_message = "Press space to scroll down...\n";
                if (write(STDOUT_FILENO, pause_message, strlen(pause_message)) < 0)
                {
                    fprintf(stderr, "ERROR write: %s\n", strerror(errno));
                    break;
                }
            }
        }

        if (is_paused && FD_ISSET(STDIN_FILENO, &read_fds))
        {
            char user_input;
            if (read(STDIN_FILENO, &user_input, 1) == 1 && user_input == ' ')
            {
                is_paused = 0;
            }
        }

        if (connection_closed && read_pos == write_pos && leftover_len == 0)
        {
            break;
        }
    }

    close(connection_socket);
    free(hostname);
    free(port_str);
    free(path);
    exit(EXIT_SUCCESS);
}
