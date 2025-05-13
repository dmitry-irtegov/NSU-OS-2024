#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include "proxy.h"
#include "cache.h"
#include "utils.h"

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
        return -1;
    }
    return 0;
}

char *read_full_request(int client_fd, size_t *out_length) {
    char *buffer = malloc(MAX_HEADER_SIZE);
    if (!buffer) return NULL;

    size_t total = 0;
    int max_timeout_ms = 5000;
    int poll_timeout = 1000;
    int elapsed_time = 0;

    while (total < MAX_HEADER_SIZE - 1 && elapsed_time < max_timeout_ms) {
        struct pollfd pfd = { .fd = client_fd, .events = POLLIN };
        int poll_res = poll(&pfd, 1, poll_timeout);
        if (poll_res == 0) {
            elapsed_time += poll_timeout;
            continue;
        } else if (poll_res < 0) {
            perror("poll error");
            free(buffer);
            return NULL;
        }

        ssize_t n = read(client_fd, buffer + total, MAX_HEADER_SIZE - total - 1);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            perror("read error");
            free(buffer);
            return NULL;
        }
        if (n == 0) break;

        total += n;
        buffer[total] = '\0';

        if (strstr(buffer, "\r\n\r\n")) break;
    }

    if (elapsed_time >= max_timeout_ms) {
        fprintf(stderr, "read_full_request: timeout waiting for client data\n");
        free(buffer);
        return NULL;
    }

    buffer[total] = '\0';
    *out_length = total;
    return buffer;
}


void process_request(int client_fd, const char *raw_request, size_t request_length) {
    if (!raw_request) return;

    char *cache_key = make_cache_key(raw_request);
    if (!cache_key) return;

    const char *cached = get_from_cache(cache_key);
    if (cached) {
        write(client_fd, cached, strlen(cached));
        printf("Cache hit for %s\n", cache_key);
        free(cache_key);
        return;
    }

    printf("Cache miss for %s\n", cache_key);
    print_cache();

    char *hostname = extract_host((char *)raw_request);
    if (!hostname) {
        free(cache_key);
        return;
    }

    struct hostent *host_entry = gethostbyname(hostname);
    if (!host_entry) {
        perror("gethostbyname");
        free(hostname);
        free(cache_key);
        return;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        free(hostname);
        free(cache_key);
        return;
    }
    set_nonblocking(server_fd);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        if (errno != EINPROGRESS) {
            close(server_fd);
            free(hostname);
            free(cache_key);
            return;
        }

        struct pollfd pfd = { .fd = server_fd, .events = POLLOUT };
        int ret = poll(&pfd, 1, 5000);
        if (ret <= 0 || !(pfd.revents & POLLOUT)) {
            perror("poll connect timeout");
            close(server_fd);
            free(hostname);
            free(cache_key);
            return;
        }

        int err;
        socklen_t len = sizeof(err);
        if (getsockopt(server_fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
            fprintf(stderr, "connect error: %s\n", strerror(err));
            close(server_fd);
            free(hostname);
            free(cache_key);
            return;
        }
    }
    printf("Connected to %s\n", hostname);

    size_t sent_total = 0;
    while (sent_total < request_length) {
        ssize_t sent = send(server_fd, raw_request + sent_total, request_length - sent_total, 0);
        if (sent > 0) {
            sent_total += (size_t)sent;
        } else if (sent < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                struct pollfd pfd = { .fd = server_fd, .events = POLLOUT };
                if (poll(&pfd, 1, -1) < 0) break;
                continue;
            }
            break;
        } else {
            break;
        }
    }

    printf("Request sent to %s\n", hostname);

    size_t capacity = 16384;
    size_t size = 0;
    char *response = malloc(capacity);
    if (!response) {
        close(server_fd);
        free(hostname);
        free(cache_key);
        return;
    }

    int max_timeout_ms = 5000;
    int poll_timeout = 1000;
    int elapsed_time = 0;
    
    while (elapsed_time < max_timeout_ms) {
        struct pollfd pfd = { .fd = server_fd, .events = POLLIN };
        int poll_res = poll(&pfd, 1, poll_timeout);
    
        if (poll_res < 0) {
            perror("poll recv");
            break;
        } else if (poll_res == 0) {
            elapsed_time += poll_timeout;
            continue;
        }
    
        if (pfd.revents & POLLIN) {
            char chunk[4096];
            ssize_t r = recv(server_fd, chunk, sizeof(chunk), 0);
            if (r < 0) {
                perror("recv");
                break;
            }
            if (r == 0) break;
    
            write(client_fd, chunk, (size_t)r);
    
            if (size + (size_t)r + 1 > capacity) {
                capacity = capacity * 2 + (size_t)r;
                char *tmp = realloc(response, capacity);
                if (!tmp) break;
                response = tmp;
            }
            memcpy(response + size, chunk, (size_t)r);
            size += (size_t)r;
    
            elapsed_time = 0;
        }
    }
    
    if (elapsed_time >= max_timeout_ms) {
        fprintf(stderr, "recv timeout from server\n");
    }
    
    response[size] = '\0';

    printf("Response received from %s\n", hostname);

    int age = should_cache_response((char *)raw_request);
    int status = extract_status_code(response);
    if (age > 0 && status >= 200 && status < 400) {
        add_to_cache(cache_key, response, age);
        mark_cache_entry_complete(cache_key);
    }

    printf("End of response from %s\n", hostname);
    
    clean_expired();
    printf("Expired cache cleaned\n");

    free(response);
    close(server_fd);
    free(hostname);
    free(cache_key);
}

int handle_client_request(int client_fd) {
    set_nonblocking(client_fd);
    size_t length = 0;
    char *req = read_full_request(client_fd, &length);
    if (!req) return 0;

    if (strcmp(req, "CLOSE_CONNECTION") == 0) {
        free(req);
        return 0;
    }

    if (!should_keep_connection(req)) {
        const char *msg = "HTTP/1.0 405 Method Not Allowed\r\nConnection: close\r\n\r\nMethod Not Allowed";
        write(client_fd, msg, strlen(msg));
        free(req);
        return 0;
    }

    process_request(client_fd, req, length);
    free(req);
    return 0;
}

char *make_cache_key(const char *raw_request) {
    char method[16] = {0}, uri[1024] = {0};
    if (sscanf(raw_request, "%15s %1023s", method, uri) != 2) {
        fprintf(stderr, "make_cache_key: invalid request\n");
        return NULL;
    }

    char *host = extract_host((char*)raw_request);
    if (!host) {
        fprintf(stderr, "make_cache_key: cannot extract host\n");
        return NULL;
    }

    char *path = extract_path((char*)raw_request);
    if (!path) {
        fprintf(stderr, "make_cache_key: cannot extract uri\n");
        free(host);
        return NULL;
    }

    printf("make_cache_key: method: %s, host: %s, path: %s\n", method, host, path);

    size_t needed = strlen(method)
                    + 8
                    + strlen(host)
                    + strlen(path)
                    + 1;

    char *key = malloc(needed);
    if (!key) {
        perror("make_cache_key: malloc");
        free(host);
        free(path);
        return NULL;
    }

    int wrote = snprintf(key, needed, "%s http://%s%s", method, host, path);
    if (wrote < 0 || (size_t)wrote >= needed) {
        fprintf(stderr, "make_cache_key: snprintf failed or truncated\n");
        free(key);
        free(host);
        free(path);
        return NULL;
    }

    free(host);
    free(path);
    return key;
}
