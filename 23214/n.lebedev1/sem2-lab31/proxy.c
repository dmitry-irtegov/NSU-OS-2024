#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <locale.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define PORT 7671
#define CACHE_SIZE 10
#define BUFFER_SIZE 1024
#define ADDRESS_SIZE 1024
#define MAX_CLIENTS 10
#define TIMEOUT 1
#define EMPTY -1
#define CACHE_TIMEOUT 60
#define DOWNLOAD_TIMEOUT 10
#define MAX_PAGE_SIZE (100 * 1024 * 1024)

typedef struct cache {
    int page_size;
    char *title;
    char *page;
    time_t created_time;
    int client_count;
    int is_complete;
    time_t download_timeout; // Time to stop downloading after last client (10s)
    time_t cache_timeout;    // Time to clear cache (60s after download_timeout)
    int server_fd;
} cache_t;

typedef struct url {
    char *host;
    char *path;
    int port;
} url_t;

typedef struct connection {
    int client_fd;
    int server_fd;
    int cache_index;
    int offset;
    char *buffer;
    int buffer_size;
    time_t last_client_time;
    char *request;
    int request_len;
    int request_sent;
} connection_t;

int socket_d = -1;
cache_t *cache = NULL;
struct pollfd fds[2 * MAX_CLIENTS + 1];
int is_client[2 * MAX_CLIENTS + 1]; // 1 for client, 0 for server, -1 for unused
connection_t connections[MAX_CLIENTS];

void signal_handler(int sig);
int findFreeIndex();
void set_nonblocking(int fd);
int acceptClient(int socket);
int tryFindAtCache(cache_t *cache, int cacheSize, char *url);
void freeURL(url_t *url);
void clientDisconnect(int index, struct pollfd *fds);
url_t *parseURL(char *urlBuff);
int async_connect(int sock, char *host, int port);
char *createRequest(const url_t *url);
void sendRequest(int server_fd, url_t *url, connection_t *conn);
int add_to_cache(cache_t *cache, char *address);
void add_to_fds(int fd, int is_client_flag, short events, struct pollfd *fds);
void handle_new_client(int socket_d, struct pollfd *fds);
void handle_client_read(int index, struct pollfd *fds, cache_t *cache);
void handle_server_read(int cache_idx, cache_t *cache, struct pollfd *fds);
void write_to_client(int index, struct pollfd *fds, cache_t *cache);
void cleanup_cache(cache_t *cache);

void signal_handler(int sig __attribute__((unused))) {
    printf("[DEBUG]: Signal received, cleaning up...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i].client_fd != EMPTY) close(connections[i].client_fd);
        free(connections[i].buffer);
        free(connections[i].request);
    }
    if (socket_d != -1) close(socket_d);
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].title) free(cache[i].title);
        if (cache[i].page) free(cache[i].page);
        if (cache[i].server_fd != EMPTY) close(cache[i].server_fd);
    }
    free(cache);
    exit(0);
}

int findFreeIndex() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i].client_fd == EMPTY) return i;
    }
    printf("[DEBUG]: No free client slots\n");
    return -1;
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
    }
}

int acceptClient(int socket) {
    int index = findFreeIndex();
    if (index == -1) return -1;
    connections[index].client_fd = accept(socket, NULL, NULL);
    if (connections[index].client_fd == -1) {
        perror("accept");
        return -1;
    }
    set_nonblocking(connections[index].client_fd);
    connections[index].server_fd = EMPTY;
    connections[index].cache_index = EMPTY;
    connections[index].offset = 0;
    connections[index].buffer_size = 0;
    connections[index].last_client_time = 0;
    if (connections[index].request) {
        free(connections[index].request);
        connections[index].request = NULL;
    }
    connections[index].request_len = 0;
    connections[index].request_sent = 0;

    if (!connections[index].buffer) {
        connections[index].buffer = malloc(ADDRESS_SIZE);
        if (!connections[index].buffer) {
            perror("malloc buffer");
            close(connections[index].client_fd);
            connections[index].client_fd = EMPTY;
            return -1;
        }
    }

    printf("[DEBUG]: New client at index: %d, fd: %d\n", index, connections[index].client_fd);
    return index;
}

void handle_new_client(int socket_d, struct pollfd *fds) {
    int index = acceptClient(socket_d);
    if (index == -1) return;
    add_to_fds(connections[index].client_fd, 1, POLLIN, fds);
}

int tryFindAtCache(cache_t *cache, int cacheSize, char *url) {
    for (int i = 0; i < cacheSize; i++) {
        if (cache[i].title && strcmp(cache[i].title, url) == 0) {
            printf("[DEBUG]: Page found at cache index %d\n", i);
            return i;
        }
    }
    printf("[DEBUG]: Page not found in cache\n");
    return -1;
}

void freeURL(url_t *url) {
    if (url) {
        free(url->host);
        free(url->path);
        free(url);
    }
}

void add_to_fds(int fd, int is_client_flag, short events, struct pollfd *fds) {
    for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
        if (fds[i].fd == -1) {
            fds[i].fd = fd;
            fds[i].events = events;
            is_client[i] = is_client_flag;
            printf("[DEBUG]: Added fd %d to fds at index %d (is_client: %d, events: %d)\n",
                   fd, i, is_client_flag, events);
            return;
        }
    }
    printf("[ERROR]: No free slots in fds array\n");
    close(fd);
}

void clientDisconnect(int index, struct pollfd *fds) {
    connection_t *conn = &connections[index];
    printf("[DEBUG]: Client disconnecting (fd: %d, reason: %s, cache_idx: %d)\n",
           conn->client_fd, conn->client_fd != EMPTY ? "client initiated" : "server or proxy initiated",
           conn->cache_index);

    if (conn->client_fd != EMPTY) {
        close(conn->client_fd);
        for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
            if (fds[i].fd == conn->client_fd) {
                fds[i].fd = -1;
                fds[i].events = 0;
                is_client[i] = -1;
                printf("[DEBUG]: Removed client fd %d from fds at index %d\n", conn->client_fd, i);
            }
        }
        conn->client_fd = EMPTY;
    }

    if (conn->cache_index != EMPTY) {
        cache[conn->cache_index].client_count--;
        printf("[DEBUG]: Cache %d client count decremented to %d\n",
               conn->cache_index, cache[conn->cache_index].client_count);
        if (cache[conn->cache_index].client_count == 0 && !cache[conn->cache_index].is_complete) {
            cache[conn->cache_index].download_timeout = time(NULL) + DOWNLOAD_TIMEOUT;
            printf("[DEBUG]: Last client disconnected, setting download timeout to %ld\n",
                   cache[conn->cache_index].download_timeout);
            if (cache[conn->cache_index].server_fd != EMPTY) {
                for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
                    if (fds[i].fd == cache[conn->cache_index].server_fd) {
                        fds[i].events |= POLLIN;
                        printf("[DEBUG]: Ensured server fd %d on POLLIN for cache %d\n",
                               cache[conn->cache_index].server_fd, conn->cache_index);
                        break;
                    }
                }
            }
        }
    }

    if (conn->request) {
        free(conn->request);
        conn->request = NULL;
        conn->request_len = 0;
        conn->request_sent = 0;
    }
    conn->offset = 0;
    conn->buffer_size = 0;
}

url_t *parseURL(char *urlBuff) {
    char *get = strstr(urlBuff, "GET");
    if (!get) {
        printf("[DEBUG]: No GET in request\n");
        return NULL;
    }

    char *http = strstr(urlBuff, "http://");
    if (!http) {
        printf("[DEBUG]: URL parsing fail: no http://\n");
        return NULL;
    }
    http += 7;

    url_t *url = malloc(sizeof(url_t));
    if (!url) {
        perror("malloc url");
        return NULL;
    }
    url->port = 80;

    char *host_end = http;
    while (*host_end && *host_end != ':' && *host_end != '/' && *host_end != ' ') host_end++;

    size_t host_len = host_end - http;
    url->host = malloc(host_len + 1);
    if (!url->host) {
        perror("malloc host");
        free(url);
        return NULL;
    }
    strncpy(url->host, http, host_len);
    url->host[host_len] = '\0';

    if (*host_end == ':') {
        char *port_start = host_end + 1;
        char *port_end = port_start;
        while (*port_end && isdigit(*port_end)) port_end++;
        if (port_end > port_start) {
            char port_str[6] = {0};
            strncpy(port_str, port_start, port_end - port_start);
            url->port = atoi(port_str);
            if (url->port <= 0 || url->port > 65535) {
                printf("[DEBUG]: Invalid port: %d\n", url->port);
                free(url->host);
                free(url);
                return NULL;
            }
        }
        host_end = port_end;
    }

    char *path_start = (*host_end == '/') ? host_end : NULL;
    char *path_end = path_start ? strchr(path_start, ' ') : NULL;
    if (path_start && path_end) {
        size_t path_len = path_end - path_start;
        url->path = malloc(path_len + 1);
        if (!url->path) {
            perror("malloc path");
            free(url->host);
            free(url);
            return NULL;
        }
        strncpy(url->path, path_start, path_len);
        url->path[path_len] = '\0';
    } else {
        url->path = malloc(2);
        if (!url->path) {
            perror("malloc path");
            free(url->host);
            free(url);
            return NULL;
        }
        strcpy(url->path, "/");
    }

    printf("[DEBUG]: Parsed host = %s, path = %s, port = %d\n", url->host, url->path, url->port);
    return url;
}

int async_connect(int sock, char *host, int port) {
    struct hostent *hp = gethostbyname(host);
    if (!hp) {
        perror("gethostbyname");
        return -1;
    }

    struct sockaddr_in addr;
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1 && errno != EINPROGRESS) {
        perror("connect");
        return -1;
    }
    return 0;
}

char *createRequest(const url_t *url) {
    char *request = malloc(ADDRESS_SIZE);
    if (!request) {
        perror("malloc request");
        return NULL;
    }
    snprintf(request, ADDRESS_SIZE, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
             url->path, url->host);
    printf("[DEBUG]: Created request: %s\n", request);
    return request;
}

void sendRequest(int server_fd, url_t *url, connection_t *conn) {
    char *request = createRequest(url);
    if (!request) return;
    conn->request = request;
    conn->request_len = strlen(request);
    conn->request_sent = 0;
    printf("[DEBUG]: Stored request for server fd %d: %s\n", server_fd, request);
}

int add_to_cache(cache_t *cache, char *address) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].title) {
            cache[i].title = strdup(address);
            if (!cache[i].title) {
                perror("malloc cache title");
                return -1;
            }
            cache[i].page = malloc(BUFFER_SIZE);
            if (!cache[i].page) {
                perror("malloc cache page");
                free(cache[i].title);
                cache[i].title = NULL;
                return -1;
            }
            cache[i].page_size = 0;
            cache[i].created_time = time(NULL);
            cache[i].client_count = 0;
            cache[i].is_complete = 0;
            cache[i].download_timeout = 0;
            cache[i].cache_timeout = 0;
            cache[i].server_fd = EMPTY;
            return i;
        }
    }
    int oldest_index = -1;
    time_t oldest_time = 0;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].client_count == 0) {
            if (oldest_index == -1 || cache[i].created_time < oldest_time) {
                oldest_index = i;
                oldest_time = cache[i].created_time;
            }
        }
    }
    if (oldest_index != -1) {
        printf("[DEBUG]: Evicting cache entry %d (title: %s)\n", oldest_index, cache[oldest_index].title);
        free(cache[oldest_index].title);
        free(cache[oldest_index].page);
        if (cache[oldest_index].server_fd != EMPTY) {
            close(cache[oldest_index].server_fd);
            for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                if (fds[k].fd == cache[oldest_index].server_fd) {
                    fds[k].fd = -1;
                    fds[k].events = 0;
                    is_client[k] = -1;
                    printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                           cache[oldest_index].server_fd, k);
                }
            }
        }

        cache[oldest_index].title = strdup(address);
        if (!cache[oldest_index].title) {
            perror("malloc cache title");
            return -1;
        }
        cache[oldest_index].page = malloc(BUFFER_SIZE);
        if (!cache[oldest_index].page) {
            perror("malloc cache page");
            free(cache[oldest_index].title);
            cache[oldest_index].title = NULL;
            return -1;
        }
        cache[oldest_index].page_size = 0;
        cache[oldest_index].created_time = time(NULL);
        cache[oldest_index].client_count = 0;
        cache[oldest_index].is_complete = 0;
        cache[oldest_index].download_timeout = 0;
        cache[oldest_index].cache_timeout = 0;
        cache[oldest_index].server_fd = EMPTY;
        return oldest_index;
    } else {
        printf("[DEBUG]: Cache full and all entries have clients\n");
        return -1;
    }
}

void handle_client_read(int index, struct pollfd *fds, cache_t *cache) {
    connection_t *conn = &connections[index];
    if (!conn->buffer) {
        printf("[ERROR]: Buffer not allocated for client fd %d\n", conn->client_fd);
        clientDisconnect(index, fds);
        return;
    }

    char buff[ADDRESS_SIZE];
    int read_bytes = read(conn->client_fd, buff, ADDRESS_SIZE - conn->buffer_size - 1);
    if (read_bytes <= 0) {
        if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            printf("[DEBUG]: Read pending for client fd %d\n", conn->client_fd);
            return;
        }
        clientDisconnect(index, fds);
        return;
    }
    memcpy(conn->buffer + conn->buffer_size, buff, read_bytes);
    conn->buffer_size += read_bytes;
    conn->buffer[conn->buffer_size] = '\0';

    if (strncmp(conn->buffer, "GET ", 4) != 0) {
        printf("[DEBUG]: Invalid request, not a GET: %s\n", conn->buffer);
        clientDisconnect(index, fds);
        return;
    }

    if (strstr(conn->buffer, "\r\n\r\n") == NULL) {
        printf("[DEBUG]: Partial request received, waiting for more data\n");
        return;
    }

    printf("[DEBUG]: Received request: %s\n", conn->buffer);
    url_t *url = parseURL(conn->buffer);
    if (!url) {
        printf("[DEBUG]: Failed to parse URL, closing connection\n");
        clientDisconnect(index, fds);
        return;
    }

    char *address = malloc(strlen(url->host) + strlen(url->path) + 2);
    if (!address) {
        perror("malloc address");
        freeURL(url);
        clientDisconnect(index, fds);
        return;
    }
    sprintf(address, "%s/%s", url->host, url->path);

    int cache_idx = tryFindAtCache(cache, CACHE_SIZE, address);
    if (cache_idx == -1) {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("socket");
            free(address);
            freeURL(url);
            clientDisconnect(index, fds);
            return;
        }
        set_nonblocking(server_fd);
        if (async_connect(server_fd, url->host, url->port) == -1) {
            close(server_fd);
            free(address);
            freeURL(url);
            clientDisconnect(index, fds);
            return;
        }
        sendRequest(server_fd, url, conn);
        add_to_fds(server_fd, 0, POLLOUT, fds);

        cache_idx = add_to_cache(cache, address);
        if (cache_idx == -1) {
            conn->cache_index = EMPTY;
            close(server_fd);
            free(address);
            freeURL(url);
            clientDisconnect(index, fds);
            return;
        } else {
            conn->cache_index = cache_idx;
            cache[cache_idx].client_count = 1;
            cache[cache_idx].server_fd = server_fd;
        }
    } else {
        conn->cache_index = cache_idx;
        cache[cache_idx].client_count++;
        cache[cache_idx].download_timeout = 0;
        cache[cache_idx].cache_timeout = 0;
        free(address);
        for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
            if (fds[i].fd == conn->client_fd) {
                fds[i].events = POLLOUT;
                printf("[DEBUG]: Updated fd %d to POLLOUT for cached data\n", conn->client_fd);
                break;
            }
        }
        if (cache[cache_idx].is_complete) {
            printf("[DEBUG]: Cache %d is complete, writing data for fd %d\n", cache_idx, conn->client_fd);
            write_to_client(index, fds, cache);
        }
    }
    freeURL(url);
    conn->buffer_size = 0;
}

void handle_server_read(int cache_idx, cache_t *cache, struct pollfd *fds) {
    if (cache_idx < 0 || cache_idx >= CACHE_SIZE || cache[cache_idx].server_fd == EMPTY) {
        printf("[ERROR]: Invalid cache index %d or empty server_fd for server read\n", cache_idx);
        return;
    }

    char buffer[BUFFER_SIZE];
    int read_bytes = read(cache[cache_idx].server_fd, buffer, BUFFER_SIZE);
    if (read_bytes <= 0) {
        if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            printf("[DEBUG]: Read pending for server fd %d\n", cache[cache_idx].server_fd);
            return;
        }
        if (read_bytes == 0) {
            cache[cache_idx].is_complete = 1;
            printf("[DEBUG]: Cache %d marked complete, page_size=%d\n", cache_idx, cache[cache_idx].page_size);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (connections[i].cache_index == cache_idx && connections[i].client_fd != EMPTY) {
                    for (int j = 1; j < 2 * MAX_CLIENTS + 1; j++) {
                        if (fds[j].fd == connections[i].client_fd) {
                            fds[j].events |= POLLOUT;
                            printf("[DEBUG]: Updated fd %d to POLLOUT for completed cache %d\n",
                                   connections[i].client_fd, cache_idx);
                            break;
                        }
                    }
                }
            }
            if (cache[cache_idx].client_count == 0) {
                printf("[DEBUG]: No clients for completed cache %d, closing server fd %d\n",
                       cache_idx, cache[cache_idx].server_fd);
                close(cache[cache_idx].server_fd);
                for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
                    if (fds[i].fd == cache[cache_idx].server_fd) {
                        fds[i].fd = -1;
                        fds[i].events = 0;
                        is_client[i] = -1;
                        printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                               cache[cache_idx].server_fd, i);
                        break;
                    }
                }
                cache[cache_idx].server_fd = EMPTY;
            }
        } else {
            printf("[ERROR]: Server read failed for fd %d: %s\n", cache[cache_idx].server_fd, strerror(errno));
            close(cache[cache_idx].server_fd);
            for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
                if (fds[i].fd == cache[cache_idx].server_fd) {
                    fds[i].fd = -1;
                    fds[i].events = 0;
                    is_client[i] = -1;
                    printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                           cache[cache_idx].server_fd, i);
                    break;
                }
            }
            cache[cache_idx].server_fd = EMPTY;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (connections[i].cache_index == cache_idx) {
                    clientDisconnect(i, fds);
                }
            }
        }
        return;
    }

    printf("[DEBUG]: Read %d bytes from server fd %d\n", read_bytes, cache[cache_idx].server_fd);
    if (cache[cache_idx].page_size + read_bytes >= MAX_PAGE_SIZE) {
        printf("[DEBUG]: Cache %d exceeds %d bytes, 100 Mb expired.\n",
               cache_idx, MAX_PAGE_SIZE);
        free(cache[cache_idx].title);
        free(cache[cache_idx].page);
        cache[cache_idx].title = NULL;
        cache[cache_idx].page = NULL;
        cache[cache_idx].page_size = 0;
        cache[cache_idx].is_complete = 0;
        close(cache[cache_idx].server_fd);
        for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
            if (fds[i].fd == cache[cache_idx].server_fd) {
                fds[i].fd = -1;
                fds[i].events = 0;
                is_client[i] = -1;
                printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                       cache[cache_idx].server_fd, i);
                break;
            }
        }
        cache[cache_idx].server_fd = EMPTY;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (connections[i].cache_index == cache_idx) {
                clientDisconnect(i, fds);
            }
        }
        return;
    } else {
        char *new_page = realloc(cache[cache_idx].page, cache[cache_idx].page_size + read_bytes + BUFFER_SIZE);
        if (!new_page) {
            printf("[ERROR]: Failed to realloc cache page for cache %d\n", cache_idx);
            free(cache[cache_idx].page);
            cache[cache_idx].page = NULL;
            cache[cache_idx].page_size = 0;
            close(cache[cache_idx].server_fd);
            for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
                if (fds[i].fd == cache[cache_idx].server_fd) {
                    fds[i].fd = -1;
                    fds[i].events = 0;
                    is_client[i] = -1;
                    printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                           cache[cache_idx].server_fd, i);
                    break;
                }
            }
            cache[cache_idx].server_fd = EMPTY;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (connections[i].cache_index == cache_idx) {
                    clientDisconnect(i, fds);
                }
            }
            return;
        }
        cache[cache_idx].page = new_page;
        memcpy(cache[cache_idx].page + cache[cache_idx].page_size, buffer, read_bytes);
        cache[cache_idx].page_size += read_bytes;
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i].cache_index == cache_idx && connections[i].client_fd != EMPTY) {
            for (int j = 1; j < 2 * MAX_CLIENTS + 1; j++) {
                if (fds[j].fd == connections[i].client_fd) {
                    fds[j].events |= POLLOUT;
                    printf("[DEBUG]: Updated fd %d to POLLOUT for new data in cache %d\n",
                           connections[i].client_fd, cache_idx);
                    break;
                }
            }
        }
    }
}

void write_to_client(int index, struct pollfd *fds, cache_t *cache) {
    connection_t *conn = &connections[index];
    int cache_idx = conn->cache_index;
    char *data;
    int size;

    if (cache_idx == -1) {
        data = conn->buffer;
        size = conn->buffer_size;
    } else {
        if (!cache[cache_idx].page) {
            printf("[ERROR]: Cache %d has null page\n", cache_idx);
            clientDisconnect(index, fds);
            return;
        }
        data = cache[cache_idx].page + conn->offset;
        size = cache[cache_idx].page_size - conn->offset;
        if (size > BUFFER_SIZE) {
            size = BUFFER_SIZE;
        }
        if (size > 0) {
            printf("[DEBUG]: Cache %d page_size=%d, offset=%d, available=%d\n",
                   cache_idx, cache[cache_idx].page_size, conn->offset, size);
        }
    }

    if (size <= 0) {
        printf("[DEBUG]: No data to write for client fd %d (cache_idx=%d, is_complete=%d)\n",
               conn->client_fd, cache_idx, cache_idx != -1 ? cache[cache_idx].is_complete : 0);
        if (cache_idx != -1 && cache[cache_idx].is_complete && conn->offset >= cache[cache_idx].page_size) {
            printf("[DEBUG]: Cache %d complete, disconnecting client fd %d\n", cache_idx, conn->client_fd);
            clientDisconnect(index, fds);
        } else {
            for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
                if (fds[i].fd == conn->client_fd) {
                    fds[i].events = 0;
                    printf("[DEBUG]: Cleared POLLOUT for client fd %d (no data)\n", conn->client_fd);
                    break;
                }
            }
        }
        return;
    }

    int written = write(conn->client_fd, data, size);
    if (written <= 0) {
        if (written == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            printf("[DEBUG]: Write pending for client fd %d\n", conn->client_fd);
            return;
        }
        printf("[ERROR]: Write failed for client fd %d: %s (cache_idx=%d, offset=%d, page_size=%d)\n",
               conn->client_fd, strerror(errno), cache_idx, conn->offset,
               cache_idx != -1 ? cache[cache_idx].page_size : 0);
        clientDisconnect(index, fds);
        return;
    }

    printf("[DEBUG]: Wrote %d bytes to client fd %d\n", written, conn->client_fd);
    conn->offset += written;

    if (cache_idx != -1 && conn->offset >= cache[cache_idx].page_size && cache[cache_idx].is_complete) {
        printf("[DEBUG]: Cache %d complete, disconnecting client fd %d\n", cache_idx, conn->client_fd);
        clientDisconnect(index, fds);
    } else {
        for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
            if (fds[i].fd == conn->client_fd) {
                fds[i].events = POLLOUT;
                printf("[DEBUG]: Kept fd %d on POLLOUT for remaining data\n", conn->client_fd);
                break;
            }
        }
    }
}

void cleanup_cache(cache_t *cache) {
    time_t now = time(NULL);
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].title || cache[i].client_count > 0) continue;

        if (cache[i].is_complete && difftime(now, cache[i].created_time) > CACHE_TIMEOUT) {
            free(cache[i].title);
            free(cache[i].page);
            cache[i].title = NULL;
            cache[i].page = NULL;
            cache[i].page_size = 0;
            cache[i].is_complete = 0;
            cache[i].download_timeout = 0;
            cache[i].cache_timeout = 0;
            if (cache[i].server_fd != EMPTY) {
                close(cache[i].server_fd);
                for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                    if (fds[k].fd == cache[i].server_fd) {
                        fds[k].fd = -1;
                        fds[k].events = 0;
                        is_client[k] = -1;
                        printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                               cache[i].server_fd, k);
                    }
                }
                cache[i].server_fd = EMPTY;
            }
            printf("[DEBUG]: Cache entry %d cleared (completed)\n", i);
        } else if (!cache[i].is_complete && cache[i].download_timeout > 0) {
            if (difftime(now, cache[i].download_timeout) > 0) {
                free(cache[i].title);
                free(cache[i].page);
                cache[i].title = NULL;
                cache[i].page = NULL;
                cache[i].page_size = 0;
                cache[i].is_complete = 0;
                cache[i].download_timeout = 0;
                cache[i].cache_timeout = 0;
                if (cache[i].server_fd != EMPTY) {
                    close(cache[i].server_fd);
                    for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                        if (fds[k].fd == cache[i].server_fd) {
                            fds[k].fd = -1;
                            fds[k].events = 0;
                            is_client[k] = -1;
                            printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                                   cache[i].server_fd, k);
                        }
                    }
                    cache[i].server_fd = EMPTY;
                }
                printf("[DEBUG]: Cache entry %d cleared (completed) because timeout expired\n", i);
            }
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal SIGINT");
        exit(1);
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal SIGPIPE");
        exit(1);
    }

    socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_d == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(socket_d, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(socket_d, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(socket_d, 2 * MAX_CLIENTS) == -1) {
        perror("listen");
        exit(1);
    }

    cache = malloc(sizeof(cache_t) * CACHE_SIZE);
    if (!cache) {
        perror("malloc cache");
        exit(1);
    }
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i] = (cache_t){0, NULL, NULL, 0, 0, 0, 0, 0, EMPTY};
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        connections[i].client_fd = EMPTY;
        connections[i].cache_index = EMPTY;
        connections[i].offset = 0;
        connections[i].buffer = malloc(ADDRESS_SIZE);
        if (!connections[i].buffer) {
            perror("malloc buffer");
            signal_handler(0);
        }
        connections[i].buffer_size = 0;
        connections[i].last_client_time = 0;
        connections[i].request = NULL;
        connections[i].request_len = 0;
        connections[i].request_sent = 0;
    }

    fds[0].fd = socket_d;
    fds[0].events = POLLIN;
    is_client[0] = -1;
    for (int i = 1; i < 2 * MAX_CLIENTS + 1; i++) {
        fds[i].fd = -1;
        fds[i].events = 0;
        is_client[i] = -1;
    }

    time_t purgeTime = time(NULL);

    while (1) {
        int num_clients = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (connections[i].client_fd != EMPTY) {
                num_clients++;
            }
        }
        int num_servers = 0;
        for (int i = 0; i < CACHE_SIZE; i++) {
            if (cache[i].server_fd != EMPTY) {
                num_servers++;
            }
        }

        if (num_clients < MAX_CLIENTS && num_servers < CACHE_SIZE) {
            fds[0].events = POLLIN;
        } else {
            fds[0].events = 0;
        }
        time_t now = time(NULL);
        if (difftime(now, purgeTime) >= TIMEOUT) {
            cleanup_cache(cache);
            purgeTime = now;
        }
        int ret = poll(fds, 2 * MAX_CLIENTS + 1, TIMEOUT * 1000);
        if (ret == -1) {
            if (errno != EINTR) {
                perror("poll");
                break;
            }
            continue;
        }
        if (ret == 0) {
            continue;
        }
        for (int i = 0; i < 2 * MAX_CLIENTS + 1; i++) {
            if (fds[i].fd == -1) continue;
            if (fds[i].revents & POLLIN) {
                if (i == 0) {
                    handle_new_client(socket_d, fds);
                } else if (is_client[i] == 1) {
                    int index = -1;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (connections[j].client_fd == fds[i].fd) {
                            index = j;
                            break;
                        }
                    }
                    if (index != -1) {
                        handle_client_read(index, fds, cache);
                    }
                } else if (is_client[i] == 0) {
                    int cache_idx = -1;
                    for (int j = 0; j < CACHE_SIZE; j++) {
                        if (cache[j].server_fd == fds[i].fd) {
                            cache_idx = j;
                            break;
                        }
                    }
                    if (cache_idx != -1) {
                        handle_server_read(cache_idx, cache, fds);
                    } else {
                        printf("[ERROR]: No cache found for server fd %d\n", fds[i].fd);
                        for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                            if (fds[k].fd == fds[i].fd) {
                                fds[k].fd = -1;
                                fds[k].events = 0;
                                is_client[k] = -1;
                                printf("[DEBUG]: Removed orphaned server fd %d from fds at index %d\n",
                                       fds[i].fd, k);
                            }
                        }
                        close(fds[i].fd);
                    }
                }
            } else if (fds[i].revents & POLLOUT) {
                if (is_client[i] == 0) {
                    int cache_idx = -1;
                    for (int j = 0; j < CACHE_SIZE; j++) {
                        if (cache[j].server_fd == fds[i].fd) {
                            cache_idx = j;
                            break;
                        }
                    }
                    if (cache_idx != -1) {
                        int error = 0;
                        socklen_t len = sizeof(error);
                        getsockopt(cache[cache_idx].server_fd, SOL_SOCKET, SO_ERROR, &error, &len);
                        if (error != 0) {
                            printf("[DEBUG]: Connection failed for server fd %d, error: %s\n",
                                   fds[i].fd, strerror(error));
                            close(cache[cache_idx].server_fd);
                            for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                                if (fds[k].fd == cache[cache_idx].server_fd) {
                                    fds[k].fd = -1;
                                    fds[k].events = 0;
                                    is_client[k] = -1;
                                    printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                                           cache[cache_idx].server_fd, k);
                                }
                            }
                            cache[cache_idx].server_fd = EMPTY;
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (connections[j].cache_index == cache_idx) {
                                    clientDisconnect(j, fds);
                                }
                            }
                        } else {
                            printf("[DEBUG]: Connection established for server fd %d\n", fds[i].fd);
                            connection_t *conn = NULL;
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (connections[j].cache_index == cache_idx && connections[j].client_fd != EMPTY) {
                                    conn = &connections[j];
                                    break;
                                }
                            }
                            if (conn && conn->request && conn->request_sent < conn->request_len) {
                                int bytes = write(cache[cache_idx].server_fd, conn->request + conn->request_sent,
                                                  conn->request_len - conn->request_sent);
                                if (bytes == -1) {
                                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                        printf("[DEBUG]: Write pending for server fd %d\n", cache[cache_idx].server_fd);
                                        continue;
                                    }
                                    perror("write request");
                                    close(cache[cache_idx].server_fd);
                                    for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                                        if (fds[k].fd == cache[cache_idx].server_fd) {
                                            fds[k].fd = -1;
                                            fds[k].events = 0;
                                            is_client[k] = -1;
                                            printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                                                   cache[cache_idx].server_fd, k);
                                        }
                                    }
                                    cache[cache_idx].server_fd = EMPTY;
                                    for (int j = 0; j < MAX_CLIENTS; j++) {
                                        if (connections[j].cache_index == cache_idx) {
                                            clientDisconnect(j, fds);
                                        }
                                    }
                                } else {
                                    conn->request_sent += bytes;
                                    printf("[DEBUG]: Sent %d/%d bytes of request for server fd %d\n",
                                           conn->request_sent, conn->request_len, cache[cache_idx].server_fd);
                                    if (conn->request_sent >= conn->request_len) {
                                        free(conn->request);
                                        conn->request = NULL;
                                        conn->request_len = 0;
                                        conn->request_sent = 0;
                                        fds[i].events = POLLIN;
                                    }
                                }
                            } else {
                                fds[i].events = POLLIN;
                            }
                        }
                    }
                } else if (is_client[i] == 1) {
                    int index = -1;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (connections[j].client_fd == fds[i].fd) {
                            index = j;
                            break;
                        }
                    }
                    if (index != -1) {
                        write_to_client(index, fds, cache);
                    }
                }
            } else if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                int index = -1;
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (connections[j].client_fd == fds[i].fd) {
                        index = j;
                        break;
                    }
                }
                if (index != -1) {
                    if (fds[i].revents & POLLERR) {
                        int error = 0;
                        socklen_t len = sizeof(error);
                        getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &error, &len);
                        printf("[ERROR]: POLLERR on fd %d, error: %s\n", fds[i].fd, strerror(error));
                    } else if (fds[i].revents & POLLHUP) {
                        printf("[DEBUG]: POLLHUP on fd %d, closing connection\n", fds[i].fd);
                    } else if (fds[i].revents & POLLNVAL) {
                        printf("[WARNING]: POLLNVAL on fd %d, invalid descriptor\n", fds[i].fd);
                    }
                    clientDisconnect(index, fds);
                } else {
                    int cache_idx = -1;
                    for (int j = 0; j < CACHE_SIZE; j++) {
                        if (cache[j].server_fd == fds[i].fd) {
                            cache_idx = j;
                            break;
                        }
                    }
                    if (cache_idx != -1) {
                        if (fds[i].revents & POLLERR) {
                            int error = 0;
                            socklen_t len = sizeof(error);
                            getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &error, &len);
                            printf("[ERROR]: POLLERR on server fd %d, error: %s\n", fds[i].fd, strerror(error));
                        } else if (fds[i].revents & POLLHUP) {
                            printf("[DEBUG]: POLLHUP on server fd %d, closing connection\n", fds[i].fd);
                        } else if (fds[i].revents & POLLNVAL) {
                            printf("[WARNING]: POLLNVAL on server fd %d, invalid descriptor\n", fds[i].fd);
                        }
                        close(cache[cache_idx].server_fd);
                        for (int k = 1; k < 2 * MAX_CLIENTS + 1; k++) {
                            if (fds[k].fd == cache[cache_idx].server_fd) {
                                fds[k].fd = -1;
                                fds[k].events = 0;
                                is_client[k] = -1;
                                printf("[DEBUG]: Removed server fd %d from fds at index %d\n",
                                       cache[cache_idx].server_fd, k);
                            }
                        }
                        cache[cache_idx].server_fd = EMPTY;
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (connections[j].cache_index == cache_idx) {
                                clientDisconnect(j, fds);
                            }
                        }
                    }
                }
            }
        }
    }
    signal_handler(0);
    return 0;
}