#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <aio.h>
#include <errno.h>

#define READ_BUF_SIZE 1024
#define MAX_LINES 25
#define TERMINAL_INDEX 0
#define CONNECTION_INDEX 1

const char *continue_prompt = "Press space to continue...";

struct response_buffer {
    char *response_data;
    size_t capacity;
    size_t length;

    char read_buf[READ_BUF_SIZE];
    size_t queued_bytes;
};

struct output_state {
    int line_progress;
    struct response_buffer *buffer;
    bool using_terminal;
    struct termios oldt;
};

void restore_terminal(struct output_state *output_state) {
    tcsetattr(STDIN_FILENO, TCSANOW, &output_state->oldt);
    output_state->using_terminal = false;
}

int transfer_queued(struct response_buffer *buffer, struct aiocb *connection_aiocb, struct aiocb **aiocb_list) {
    if (buffer->queued_bytes == 0) {
        return 0;
    }

    if (buffer->length + buffer->queued_bytes > buffer->capacity) {
        size_t new_capacity = (buffer->length + buffer->queued_bytes) * 2;
        char *new_data = realloc(buffer->response_data, new_capacity);
        if (new_data == NULL) {
            return -1;
        }
        buffer->response_data = new_data;
        buffer->capacity = new_capacity;
    }

    memcpy(buffer->response_data + buffer->length, buffer->read_buf, buffer->queued_bytes);
    buffer->length += buffer->queued_bytes;
    buffer->queued_bytes = 0;


    if (aio_read(connection_aiocb) == 0) {
        aiocb_list[CONNECTION_INDEX] = connection_aiocb;
    } else {
        perror("Failed to read from socket");
    }

    return 0;
}

void update_output(
    struct output_state *output_state, 
    struct aiocb *terminal_aiocb, 
    struct aiocb *connection_aiocb, 
    struct aiocb **aiocb_list
) {
    if (aiocb_list[TERMINAL_INDEX] != NULL) {
        return;
    }

    size_t line_beginning = 0;
    for (size_t i = 0; i < output_state->buffer->length; i++) {
        if (output_state->buffer->response_data[i] == '\n') {
            output_state->line_progress++;
            write(
                STDOUT_FILENO, 
                &output_state->buffer->response_data[line_beginning], 
                i + 1 - line_beginning
            );
            line_beginning = i + 1;

            if (output_state->using_terminal && output_state->line_progress >= MAX_LINES ) {
                if (aio_read(terminal_aiocb) == 0) {
                    aiocb_list[TERMINAL_INDEX] = terminal_aiocb;
                    fprintf(stderr, "%s", continue_prompt);

                    output_state->line_progress = 0;

                    output_state->buffer->length -= line_beginning;
                    memmove(
                        output_state->buffer->response_data, 
                        &output_state->buffer->response_data[line_beginning], 
                        output_state->buffer->length
                    );
                    
                    goto after_output;
                } else {
                    restore_terminal(output_state);
                }
            }
        }
    }

    write(
        STDOUT_FILENO, 
        &output_state->buffer->response_data[line_beginning], 
        output_state->buffer->length - line_beginning
    );
    output_state->buffer->length -= line_beginning;

    after_output:
    transfer_queued(output_state->buffer, connection_aiocb, aiocb_list);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: lab30 <URL>\n");
        return 1;
    }

    // URL parsing

    char *url = argv[1];
    char *host_start = strstr(url, "://");
    if (host_start == NULL) {
        host_start = url;
    } else {
        host_start += 3;
    }
    size_t host_len;

    char *path_start = strchr(host_start, '/');
    if (path_start == NULL) {
        host_len = strlen(host_start);
        path_start = "/";
    } else {
        host_len = path_start - host_start;
    }

    char *host = malloc(host_len + 1);
    char *path = malloc(strlen(path_start) + 1);
    if (host == NULL || path == NULL) {
        perror("Not enough memory");
        return 1;
    }
    memcpy(host, host_start, host_len);
    host[host_len] = '\0';
    strcpy(path, path_start);

    // Connect to host

    struct hostent *he = gethostbyname(host);
    if (he == NULL) {
        fprintf(stderr, "Failed to resolve host %s\n", host);
        return 1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);
    memcpy(&servaddr.sin_addr.s_addr, he->h_addr_list[0], strlen(he->h_addr_list[0]));

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Failed to connect");
        return 1;
    }

    // Send request

    const char *request = "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n";
    size_t request_len = strlen(request) + strlen(path) + strlen(host) + 1;
    char *request_buf = malloc(request_len);
    if (request_buf == NULL) {
        perror("Not enough memory");
        return 1;
    }
    snprintf(request_buf, request_len, request, path, host);
    if (write(sockfd, request_buf, request_len) < 0) {
        perror("Failed to send request");
        return 1;
    }

    free(host);
    free(path);
    free(request_buf);

    // Aio initialization
    
    struct response_buffer response_buffer;
    response_buffer.response_data = malloc(READ_BUF_SIZE * 5);
    response_buffer.capacity = READ_BUF_SIZE * 5;
    response_buffer.length = 0;
    response_buffer.queued_bytes = 0;

    if (response_buffer.response_data == NULL) {
        perror("Not enough memory");
        return 1;
    }

    char terminal_buf;

    struct aiocb terminal_aiocb;
    memset(&terminal_aiocb, 0, sizeof(struct aiocb));
    terminal_aiocb.aio_fildes = STDIN_FILENO;
    terminal_aiocb.aio_buf = &terminal_buf;
    terminal_aiocb.aio_nbytes = 1;
    terminal_aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;

    struct aiocb connection_aiocb;
    memset(&connection_aiocb, 0, sizeof(struct aiocb));
    connection_aiocb.aio_fildes = sockfd;
    connection_aiocb.aio_buf = response_buffer.read_buf;
    connection_aiocb.aio_nbytes = READ_BUF_SIZE;
    connection_aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;

    if (aio_read(&connection_aiocb) < 0) {
        perror("Failed to read from socket");
        return 1;
    }

    struct aiocb *aiocb_list[2];
    aiocb_list[TERMINAL_INDEX] = NULL;
    aiocb_list[CONNECTION_INDEX] = &connection_aiocb;

    // Initializing terminal state

    struct output_state output_state;
    output_state.line_progress = 0;
    output_state.buffer = &response_buffer;

    output_state.using_terminal = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);

    if (output_state.using_terminal) {
        struct termios newt;
        tcgetattr(STDIN_FILENO, &output_state.oldt);
        newt = output_state.oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    // Read

    while (1) {
        aio_suspend((const struct aiocb *const *)aiocb_list, 2, NULL);
        
        if (aiocb_list[TERMINAL_INDEX] != NULL) {
            int err = aio_error(&terminal_aiocb);
            if (err == EINPROGRESS) {
                goto after_terminal_read;
            }
            aiocb_list[TERMINAL_INDEX] = NULL;

            ssize_t bytes_read = aio_return(&terminal_aiocb);
            if (bytes_read > 0) {
                if (terminal_buf == ' ') {
                    fprintf(stderr, "\r%*c\r", strlen(continue_prompt), ' ');
                    update_output(&output_state, &terminal_aiocb, &connection_aiocb, aiocb_list);
                } else {
                    if (aio_read(&terminal_aiocb) == 0) {
                        aiocb_list[TERMINAL_INDEX] = &terminal_aiocb;
                    } else {
                        restore_terminal(&output_state);
                    }
                }
            } else {
                if (bytes_read < 0) {
                    fprintf(stderr, "Failed to read from terminal: %s\n", strerror(err));
                }
                restore_terminal(&output_state);

                update_output(&output_state, &terminal_aiocb, &connection_aiocb, aiocb_list);
            }
        }

        after_terminal_read:
        if (aiocb_list[CONNECTION_INDEX] != NULL) {
            int err = aio_error(&connection_aiocb);
            if (err == EINPROGRESS) {
                goto after_connection_read;
            }
            aiocb_list[CONNECTION_INDEX] = NULL;

            ssize_t bytes_read = aio_return(&connection_aiocb);
            if (bytes_read > 0) {
                response_buffer.queued_bytes = bytes_read;
                if (transfer_queued(&response_buffer, &connection_aiocb, aiocb_list) == 0) {
                    update_output(&output_state, &terminal_aiocb, &connection_aiocb, aiocb_list);
                }
            } else if (bytes_read < 0) {
                fprintf(stderr, "Failed to read from socket: %s\n", strerror(err));    
            }
        }

        after_connection_read:
        if (aiocb_list[TERMINAL_INDEX] == NULL && aiocb_list[CONNECTION_INDEX] == NULL) {
            break;
        }
    }

    if (output_state.using_terminal) {
        restore_terminal(&output_state);
    }

    return 0;
}
