#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define READ_BUF_SIZE 8192
#define MAX_LINES 25

const char *continue_prompt = "Press space to continue...";

struct response_buffer {
    char data[READ_BUF_SIZE];
    size_t head;
    size_t tail;
    bool finished;
    bool completely_filled;

    pthread_mutex_t mutex;
    pthread_cond_t updated;
};

void *terminal_thread(void *arg) {
    int line_progress = 0;
    struct response_buffer *response_buffer = (struct response_buffer *)arg;
    bool using_terminal = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    struct termios oldt;

    if (using_terminal) {
        tcgetattr(STDIN_FILENO, &oldt);
        struct termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    while (1) {
        pthread_mutex_lock(&response_buffer->mutex);
        while (response_buffer->head == response_buffer->tail && !response_buffer->completely_filled) {
            if (response_buffer->finished) {
                pthread_mutex_unlock(&response_buffer->mutex);
                if (using_terminal) {
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                }
                return NULL;
            }

            pthread_cond_wait(&response_buffer->updated, &response_buffer->mutex);
        }

        const size_t started_from_i = response_buffer->head;
        
        while (response_buffer->head != response_buffer->tail || response_buffer->completely_filled) {
            if (response_buffer->data[response_buffer->head] == '\n') {
                line_progress++;
            }

            response_buffer->head = (response_buffer->head + 1) % READ_BUF_SIZE;
            response_buffer->completely_filled = false;

            if (using_terminal && line_progress >= MAX_LINES ) {
                break;
            }
        }

        const size_t ended_on_i = response_buffer->head;

        if (started_from_i < ended_on_i) {
            write(STDOUT_FILENO, &response_buffer->data[started_from_i], ended_on_i - started_from_i);
        } else {
            write(STDOUT_FILENO, &response_buffer->data[started_from_i], READ_BUF_SIZE - started_from_i);
            write(STDOUT_FILENO, &response_buffer->data[0], ended_on_i);
        }

        pthread_cond_signal(&response_buffer->updated);
        pthread_mutex_unlock(&response_buffer->mutex);
        
        if (using_terminal && line_progress >= MAX_LINES ) {
            fprintf(stderr, "%s", continue_prompt);

            char c = '\0';
            while (c != ' ') {
                read(STDIN_FILENO, &c, 1);
            }
            fprintf(stderr, "\r%*c\r", strlen(continue_prompt), ' ');

            line_progress = 0;
        }
    }
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

    // Initialize buffer and thread
    
    struct response_buffer response_buffer;
    response_buffer.head = 0;
    response_buffer.tail = 0;
    response_buffer.finished = false;
    response_buffer.completely_filled = false;

    int err = pthread_mutex_init(&response_buffer.mutex, NULL);
    if (err != 0) {
        perror("Failed to initialize mutex");
        return 1;
    }
    err = pthread_cond_init(&response_buffer.updated, NULL);
    if (err != 0) {
        perror("Failed to initialize condition variable");
        return 1;
    }

    pthread_t tid;
    err = pthread_create(&tid, NULL, terminal_thread, (void *)&response_buffer);
    if (err != 0) {
        perror("Failed to create thread");
        return 1;
    }
    
    // Read

    size_t head = 0;
    size_t tail = 0;

    while (1) {
        size_t read_len;
        if (head > tail) {
            read_len = head - tail;
        } else {
            read_len = READ_BUF_SIZE - tail;
        }

        read_len = read(sockfd, &response_buffer.data[tail], read_len);

        pthread_mutex_lock(&response_buffer.mutex);

        if (read_len == 0) {
            response_buffer.finished = true;
            pthread_mutex_unlock(&response_buffer.mutex);
            pthread_cond_signal(&response_buffer.updated);

            break;
        }

        response_buffer.tail = (response_buffer.tail + read_len) % READ_BUF_SIZE;
        pthread_cond_signal(&response_buffer.updated);

        if (response_buffer.tail == response_buffer.head) {
            response_buffer.completely_filled = true;

            while (response_buffer.completely_filled) {
                pthread_cond_wait(&response_buffer.updated, &response_buffer.mutex);
            }
        }

        head = response_buffer.head;
        tail = response_buffer.tail;

        pthread_mutex_unlock(&response_buffer.mutex);
    }

    err = pthread_join(tid, NULL);
    if (err != 0) {
        perror("Failed to join thread");
        return 1;
    }

    return 0;
}
