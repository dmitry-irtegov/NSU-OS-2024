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
#include <pthread.h>

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
    time_t download_timeout;
    time_t cache_timeout;
    int server_fd;
    pthread_t server_thread;
    int has_server_thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} cache_t;

typedef struct url {
    char *host;
    char *path;
    int port;
} url_t;

typedef struct connection {
    int client_fd;
    int cache_index;
    int offset;
    char *buffer;
    int buffer_size;
    time_t last_client_time;
    char *request;
    int request_len;
    int request_sent;
    pthread_t client_thread;
} connection_t;

int socket_d = -1;
cache_t *cache = NULL;
connection_t connections[MAX_CLIENTS];
pthread_mutex_t connections_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t accept_cond = PTHREAD_COND_INITIALIZER;
volatile int running;

void signal_handler(int sig);
int findFreeIndex();
void set_nonblocking(int fd);
int acceptClient();
int tryFindAtCache(cache_t *cache, int cacheSize, char *url);
void freeURL(url_t *url);
url_t *parseURL(char *urlBuff);
int async_connect(int sock, char *host, int port);
char *createRequest(const url_t *url);
int add_to_cache(cache_t *cache, char *address);
void cleanup_cache(cache_t *cache);
void *client_thread(void *arg);
void *server_thread(void *arg);
void *cache_cleanup_thread(void *arg);

void signal_handler(int sig __attribute__((unused))) {
    printf("[DEBUG]: Signal received, cleaning up...\n");
    running = 0;

    pthread_mutex_lock(&accept_mutex);
    pthread_cond_broadcast(&accept_cond);
    pthread_mutex_unlock(&accept_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i].client_fd != EMPTY) {
            pthread_join(connections[i].client_thread, NULL);
        }
    }

    pthread_mutex_lock(&cache_mutex);
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].has_server_thread) {
            pthread_join(cache[i].server_thread, NULL);
            cache[i].has_server_thread = 0;
        }
        if (cache[i].title) {
            pthread_mutex_lock(&cache[i].mutex);
            free(cache[i].title);
            free(cache[i].page);
            cache[i].title = NULL;
            cache[i].page = NULL;
            if (cache[i].server_fd != EMPTY) {
                close(cache[i].server_fd);
                cache[i].server_fd = EMPTY;
            }
            pthread_cond_broadcast(&cache[i].cond);
            pthread_mutex_unlock(&cache[i].mutex);
            pthread_mutex_destroy(&cache[i].mutex);
            pthread_cond_destroy(&cache[i].cond);
        }
    }
    free(cache);
    cache = NULL;
    pthread_mutex_unlock(&cache_mutex);
    if (socket_d != -1) {
        close(socket_d);
        socket_d = -1;
    }
    pthread_mutex_destroy(&connections_mutex);
    pthread_mutex_destroy(&cache_mutex);
    pthread_mutex_destroy(&accept_mutex);
    pthread_cond_destroy(&accept_cond);
}

int findFreeIndex() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i].client_fd == EMPTY) {
            return i;
        }
    }
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

int acceptClient() {
    pthread_mutex_lock(&connections_mutex);
    int index = findFreeIndex();
    if (index == -1) {
        pthread_mutex_unlock(&connections_mutex);
        return -1;
    }
    connections[index].client_fd = accept(socket_d, NULL, NULL);
    if (connections[index].client_fd == -1) {
        perror("accept");
        pthread_mutex_unlock(&connections_mutex);
        return -1;
    }
    set_nonblocking(connections[index].client_fd);
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
            pthread_mutex_unlock(&connections_mutex);
            return -1;
        }
    }

    if (pthread_create(&connections[index].client_thread, NULL, client_thread, (void *)(long)index) != 0) {
        perror("pthread_create client");
        close(connections[index].client_fd);
        free(connections[index].buffer);
        connections[index].client_fd = EMPTY;
        pthread_mutex_unlock(&connections_mutex);
        return -1;
    }

    printf("[DEBUG]: New client at index: %d, fd: %d\n", index, connections[index].client_fd);
    pthread_mutex_unlock(&connections_mutex);
    return index;
}

int tryFindAtCache(cache_t *cache, int cacheSize, char *url) {
    pthread_mutex_lock(&cache_mutex);
    for (int i = 0; i < cacheSize; i++) {
        if (cache[i].title && strcmp(cache[i].title, url) == 0) {
            pthread_mutex_lock(&cache[i].mutex);
            cache[i].client_count++;
            pthread_mutex_unlock(&cache[i].mutex);
            pthread_mutex_unlock(&cache_mutex);
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

int add_to_cache(cache_t *cache, char *address) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].title) {
            cache[i].title = strdup(address);
            if (!cache[i].title) {
                perror("malloc cache title");
                pthread_mutex_unlock(&cache_mutex);
                return -1;
            }
            cache[i].page = malloc(BUFFER_SIZE);
            if (!cache[i].page) {
                perror("malloc cache page");
                free(cache[i].title);
                cache[i].title = NULL;
                pthread_mutex_unlock(&cache_mutex);
                return -1;
            }
            cache[i].page_size = 0;
            cache[i].created_time = time(NULL);
            cache[i].client_count = 0;
            cache[i].is_complete = 0;
            cache[i].download_timeout = 0;
            cache[i].cache_timeout = 0;
            cache[i].server_fd = EMPTY;
            cache[i].has_server_thread = 0;
            pthread_mutex_init(&cache[i].mutex, NULL);
            pthread_cond_init(&cache[i].cond, NULL);
            pthread_mutex_unlock(&cache_mutex);
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
        pthread_mutex_lock(&cache[oldest_index].mutex);
        printf("[DEBUG]: Evicting cache entry %d (title: %s)\n", oldest_index, cache[oldest_index].title);
        free(cache[oldest_index].title);
        free(cache[oldest_index].page);
        if (cache[oldest_index].server_fd != EMPTY) {
            close(cache[oldest_index].server_fd);
        }
        pthread_cond_broadcast(&cache[oldest_index].cond);
        cache[oldest_index].title = strdup(address);
        if (!cache[oldest_index].title) {
            perror("malloc cache title");
            pthread_mutex_unlock(&cache[oldest_index].mutex);
            pthread_mutex_unlock(&cache_mutex);
            return -1;
        }
        cache[oldest_index].page = malloc(BUFFER_SIZE);
        if (!cache[oldest_index].page) {
            perror("malloc cache page");
            free(cache[oldest_index].title);
            cache[oldest_index].title = NULL;
            pthread_mutex_unlock(&cache[oldest_index].mutex);
            pthread_mutex_unlock(&cache_mutex);
            return -1;
        }
        cache[oldest_index].page_size = 0;
        cache[oldest_index].created_time = time(NULL);
        cache[oldest_index].client_count = 0;
        cache[oldest_index].is_complete = 0;
        cache[oldest_index].download_timeout = 0;
        cache[oldest_index].cache_timeout = 0;
        cache[oldest_index].server_fd = EMPTY;
        cache[oldest_index].has_server_thread = 0;
        pthread_mutex_unlock(&cache[oldest_index].mutex);
        pthread_mutex_unlock(&cache_mutex);
        return oldest_index;
    } else {
        printf("[DEBUG]: Cache full and all entries have clients\n");
        pthread_mutex_unlock(&cache_mutex);
        return -1;
    }
}

void cleanup_cache(cache_t *cache) {
    time_t now = time(NULL);
    pthread_mutex_lock(&cache_mutex);
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i].title || cache[i].client_count > 0) continue;
        pthread_mutex_lock(&cache[i].mutex);
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
                cache[i].server_fd = EMPTY;
            }
            pthread_cond_broadcast(&cache[i].cond);
            pthread_mutex_unlock(&cache[i].mutex);
            pthread_mutex_destroy(&cache[i].mutex);
            pthread_cond_destroy(&cache[i].cond);
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
                    cache[i].server_fd = EMPTY;
                }
                pthread_cond_broadcast(&cache[i].cond);
                pthread_mutex_unlock(&cache[i].mutex);
                pthread_mutex_destroy(&cache[i].mutex);
                pthread_cond_destroy(&cache[i].cond);
                printf("[DEBUG]: Cache entry %d cleared (timeout expired)\n", i);
            } else {
                pthread_mutex_unlock(&cache[i].mutex);
            }
        } else {
            pthread_mutex_unlock(&cache[i].mutex);
        }
    }
    pthread_mutex_unlock(&cache_mutex);
}

void *cache_cleanup_thread(void *arg __attribute__((unused))) {
    while (running) {
        cleanup_cache(cache);
        sleep(TIMEOUT);
    }
    return NULL;
}

void *client_thread(void *arg) {
    int index = (int)(long)arg;
    connection_t *conn = &connections[index];
    struct pollfd fds[1];
    fds[0].fd = conn->client_fd;
    fds[0].events = POLLIN;

    while (running && conn->client_fd != EMPTY) {
        int ret = poll(fds, 1, TIMEOUT * 1000);
        if (!running) {
            printf("[DEBUG]: Client thread fd %d exiting due to shutdown\n", conn->client_fd);
            break;
        }
        if (ret == -1) {
            if (errno != EINTR) {
                perror("poll");
                break;
            }
            continue;
        }
        if (ret == 0) continue;

        if (fds[0].revents & POLLIN) {
            char buff[ADDRESS_SIZE];
            int read_bytes = read(conn->client_fd, buff, ADDRESS_SIZE - conn->buffer_size - 1);
            if (read_bytes <= 0) {
                if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    printf("[DEBUG]: Read pending for client fd %d\n", conn->client_fd);
                    continue;
                }
                break;
            }
            memcpy(conn->buffer + conn->buffer_size, buff, read_bytes);
            conn->buffer_size += read_bytes;
            conn->buffer[conn->buffer_size] = '\0';

            if (strncmp(conn->buffer, "GET ", 4) != 0) {
                printf("[DEBUG]: Invalid request, not a GET: %s\n", conn->buffer);
                break;
            }

            if (strstr(conn->buffer, "\r\n\r\n") == NULL) {
                printf("[DEBUG]: Partial request received, waiting for more data\n");
                continue;
            }

            printf("[DEBUG]: Received request: %s\n", conn->buffer);
            url_t *url = parseURL(conn->buffer);
            if (!url) {
                printf("[DEBUG]: Failed to parse URL, closing connection\n");
                break;
            }

            char *address = malloc(strlen(url->host) + strlen(url->path) + 2);
            if (!address) {
                perror("malloc address");
                freeURL(url);
                break;
            }
            sprintf(address, "%s/%s", url->host, url->path);

            int cache_idx = tryFindAtCache(cache, CACHE_SIZE, address);
            if (cache_idx == -1) {
                int server_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (server_fd == -1) {
                    perror("socket");
                    free(address);
                    freeURL(url);
                    break;
                }
                set_nonblocking(server_fd);
                if (async_connect(server_fd, url->host, url->port) == -1) {
                    close(server_fd);
                    free(address);
                    freeURL(url);
                    break;
                }
                char *request = createRequest(url);
                if (!request) {
                    close(server_fd);
                    free(address);
                    freeURL(url);
                    break;
                }
                conn->request = request;
                conn->request_len = strlen(request);
                conn->request_sent = 0;

                cache_idx = add_to_cache(cache, address);
                if (cache_idx == -1) {
                    close(server_fd);
                    free(address);
                    freeURL(url);
                    free(conn->request);
                    conn->request = NULL;
                    break;
                }
                pthread_mutex_lock(&cache[cache_idx].mutex);
                cache[cache_idx].client_count++;
                cache[cache_idx].server_fd = server_fd;
                if (!cache[cache_idx].has_server_thread) {
                    cache[cache_idx].has_server_thread = 1;
                    if (pthread_create(&cache[cache_idx].server_thread, NULL, server_thread, (void *)(long)cache_idx) != 0) {
                        perror("pthread_create server");
                        close(server_fd);
                        cache[cache_idx].server_fd = EMPTY;
                        cache[cache_idx].client_count--;
                        free(address);
                        freeURL(url);
                        free(conn->request);
                        conn->request = NULL;
                        pthread_mutex_unlock(&cache[cache_idx].mutex);
                        break;
                    }
                }
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                conn->cache_index = cache_idx;
            } else {
                conn->cache_index = cache_idx;
                free(address);
            }
            freeURL(url);
            conn->buffer_size = 0;
            fds[0].events = POLLOUT;
        } else if (fds[0].revents & POLLOUT) {
            int cache_idx = conn->cache_index;
            if (cache_idx == -1) {
                printf("[ERROR]: No cache index for client fd %d\n", conn->client_fd);
                break;
            }
            pthread_mutex_lock(&cache[cache_idx].mutex);
            if (!cache[cache_idx].page) {
                printf("[ERROR]: Cache %d has null page\n", cache_idx);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }

            while (cache[cache_idx].page_size - conn->offset <= 0 && !cache[cache_idx].is_complete && running) {
                printf("[DEBUG]: Waiting for data in cache %d for client fd %d\n", cache_idx, conn->client_fd);
                pthread_cond_wait(&cache[cache_idx].cond, &cache[cache_idx].mutex);
            }
            if (!running) {
                printf("[DEBUG]: Client thread fd %d exiting due to shutdown\n", conn->client_fd);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }

            if (!cache[cache_idx].page && cache[cache_idx].is_complete) {
                printf("[DEBUG]: Cache %d empty and complete, disconnecting client fd %d\n", cache_idx, conn->client_fd);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }

            char *data = cache[cache_idx].page + conn->offset;
            int size = cache[cache_idx].page_size - conn->offset;
            if (size > BUFFER_SIZE) {
                size = BUFFER_SIZE;
            }
            if (size > 0) {
                printf("[DEBUG]: Cache %d page_size=%d, offset=%d, available=%d\n",
                       cache_idx, cache[cache_idx].page_size, conn->offset, size);
            }

            if (size <= 0 && cache[cache_idx].is_complete) {
                printf("[DEBUG]: Cache %d complete, all data sent (offset=%d, page_size=%d), disconnecting client fd %d\n",
                       cache_idx, conn->offset, cache[cache_idx].page_size, conn->client_fd);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }

            pthread_mutex_unlock(&cache[cache_idx].mutex);

            int written = write(conn->client_fd, data, size);
            if (written <= 0) {
                if (written == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    printf("[DEBUG]: Write pending for client fd %d\n", conn->client_fd);
                    continue;
                }
                printf("[ERROR]: Write failed for client fd %d: %s\n", conn->client_fd, strerror(errno));
                break;
            }

            printf("[DEBUG]: Wrote %d bytes to client fd %d\n", written, conn->client_fd);
            conn->offset += written;

            pthread_mutex_lock(&cache[cache_idx].mutex);
            if (cache[cache_idx].is_complete && conn->offset >= cache[cache_idx].page_size) {
                printf("[DEBUG]: Cache %d complete, disconnecting client fd %d\n", cache_idx, conn->client_fd);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }
            pthread_mutex_unlock(&cache[cache_idx].mutex);
        } else if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            if (fds[0].revents & POLLERR) {
                int error = 0;
                socklen_t len = sizeof(error);
                getsockopt(conn->client_fd, SOL_SOCKET, SO_ERROR, &error, &len);
                printf("[ERROR]: POLLERR on client fd %d, error: %s\n", conn->client_fd, strerror(error));
            } else if (fds[0].revents & POLLHUP) {
                printf("[DEBUG]: POLLHUP on client fd %d\n", conn->client_fd);
            } else if (fds[0].revents & POLLNVAL) {
                printf("[WARNING]: POLLNVAL on client fd %d\n", conn->client_fd);
            }
            break;
        }
    }

    pthread_mutex_lock(&connections_mutex);
    int cache_idx = conn->cache_index;
    if (cache_idx != -1) {
        pthread_mutex_lock(&cache[cache_idx].mutex);
        cache[cache_idx].client_count--;
        if (cache[cache_idx].client_count == 0 && !cache[cache_idx].is_complete) {
            cache[cache_idx].download_timeout = time(NULL) + DOWNLOAD_TIMEOUT;
            printf("[DEBUG]: Last client disconnected, setting download timeout to %ld\n",
                   cache[cache_idx].download_timeout);
        }
        pthread_cond_broadcast(&cache[cache_idx].cond);
        pthread_mutex_unlock(&cache[cache_idx].mutex);
    }
    close(conn->client_fd);
    free(conn->buffer);
    free(conn->request);
    conn->client_fd = EMPTY;
    conn->buffer = NULL;
    conn->request = NULL;
    conn->cache_index = EMPTY;
    conn->offset = 0;
    conn->buffer_size = 0;
    conn->request_len = 0;
    conn->request_sent = 0;
    printf("[DEBUG]: Client fd %d disconnected\n", conn->client_fd);
    pthread_mutex_lock(&accept_mutex);
    pthread_cond_signal(&accept_cond);
    pthread_mutex_unlock(&accept_mutex);
    pthread_mutex_unlock(&connections_mutex);
    return NULL;
}

void *server_thread(void *arg) {
    int cache_idx = (int)(long)arg;
    struct pollfd fds[1];
    char *request = NULL;
    int request_len = 0;
    int request_sent = 0;

    pthread_mutex_lock(&cache[cache_idx].mutex);
    fds[0].fd = cache[cache_idx].server_fd;
    fds[0].events = POLLOUT;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i].cache_index == cache_idx && connections[i].request) {
            request = strdup(connections[i].request);
            request_len = connections[i].request_len;
            request_sent = connections[i].request_sent;
            break;
        }
    }
    pthread_mutex_unlock(&cache[cache_idx].mutex);

    if (!request) {
        printf("[ERROR]: No request found for server thread (cache %d)\n", cache_idx);
        pthread_mutex_lock(&cache[cache_idx].mutex);
        close(cache[cache_idx].server_fd);
        cache[cache_idx].server_fd = EMPTY;
        cache[cache_idx].has_server_thread = 0;
        pthread_cond_broadcast(&cache[cache_idx].cond);
        pthread_mutex_unlock(&cache[cache_idx].mutex);
        return NULL;
    }

    while (running && fds[0].fd != EMPTY) {
        int ret = poll(fds, 1, TIMEOUT * 1000);
        if (ret == -1) {
            if (errno != EINTR) {
                perror("poll");
                break;
            }
            continue;
        }
        if (ret == 0) continue;

        if (fds[0].revents & POLLOUT) {
            int error = 0;
            socklen_t len = sizeof(error);
            getsockopt(fds[0].fd, SOL_SOCKET, SO_ERROR, &error, &len);
            if (error != 0) {
                printf("[DEBUG]: Connection failed for server fd %d: %s\n", fds[0].fd, strerror(error));
                break;
            }
            if (request_sent < request_len) {
                int bytes = write(fds[0].fd, request + request_sent, request_len - request_sent);
                if (bytes == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("[DEBUG]: Write pending for server fd %d\n", fds[0].fd);
                        continue;
                    }
                    perror("write request");
                    break;
                }
                request_sent += bytes;
                printf("[DEBUG]: Sent %d/%d bytes of request for server fd %d\n",
                       request_sent, request_len, fds[0].fd);
                if (request_sent >= request_len) {
                    free(request);
                    request = NULL;
                    fds[0].events = POLLIN;
                }
            } else {
                fds[0].events = POLLIN;
            }
        } else if (fds[0].revents & POLLIN) {
            char buffer[BUFFER_SIZE];
            int read_bytes = read(fds[0].fd, buffer, BUFFER_SIZE);
            if (read_bytes <= 0) {
                if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    printf("[DEBUG]: Read pending for server fd %d\n", fds[0].fd);
                    continue;
                }
                pthread_mutex_lock(&cache[cache_idx].mutex);
                cache[cache_idx].is_complete = 1;
                printf("[DEBUG]: Cache %d marked complete, page_size=%d\n", cache_idx, cache[cache_idx].page_size);
                pthread_cond_broadcast(&cache[cache_idx].cond);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }

            pthread_mutex_lock(&cache[cache_idx].mutex);
            if (cache[cache_idx].page_size + read_bytes >= MAX_PAGE_SIZE) {
                printf("[DEBUG]: Cache %d exceeds %d bytes, clearing\n", cache_idx, MAX_PAGE_SIZE);
                free(cache[cache_idx].title);
                free(cache[cache_idx].page);
                cache[cache_idx].title = NULL;
                cache[cache_idx].page = NULL;
                cache[cache_idx].page_size = 0;
                cache[cache_idx].is_complete = 0;
                pthread_cond_broadcast(&cache[cache_idx].cond);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }

            char *new_page = realloc(cache[cache_idx].page, cache[cache_idx].page_size + read_bytes + BUFFER_SIZE);
            if (!new_page) {
                printf("[ERROR]: Failed to realloc cache page for cache %d\n", cache_idx);
                free(cache[cache_idx].page);
                cache[cache_idx].page = NULL;
                cache[cache_idx].page_size = 0;
                pthread_cond_broadcast(&cache[cache_idx].cond);
                pthread_mutex_unlock(&cache[cache_idx].mutex);
                break;
            }
            cache[cache_idx].page = new_page;
            memcpy(cache[cache_idx].page + cache[cache_idx].page_size, buffer, read_bytes);
            cache[cache_idx].page_size += read_bytes;
            printf("[DEBUG]: Read %d bytes into cache %d\n", read_bytes, cache_idx);
            pthread_cond_broadcast(&cache[cache_idx].cond);
            pthread_mutex_unlock(&cache[cache_idx].mutex);
        } else if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            if (fds[0].revents & POLLERR) {
                int error = 0;
                socklen_t len = sizeof(error);
                getsockopt(fds[0].fd, SOL_SOCKET, SO_ERROR, &error, &len);
                printf("[ERROR]: POLLERR on server fd %d: %s\n", fds[0].fd, strerror(error));
            } else if (fds[0].revents & POLLHUP) {
                printf("[DEBUG]: POLLHUP on server fd %d\n", fds[0].fd);
            } else if (fds[0].revents & POLLNVAL) {
                printf("[WARNING]: POLLNVAL on server fd %d\n", fds[0].fd);
            }
            break;
        }
    }

    pthread_mutex_lock(&cache[cache_idx].mutex);
    if (cache[cache_idx].server_fd != EMPTY) {
        close(cache[cache_idx].server_fd);
        cache[cache_idx].server_fd = EMPTY;
    }
    cache[cache_idx].has_server_thread = 0;
    pthread_cond_broadcast(&cache[cache_idx].cond);
    pthread_mutex_unlock(&cache[cache_idx].mutex);
    free(request);
    printf("[DEBUG]: Server thread for cache %d terminated\n", cache_idx);
    return NULL;
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
        cache[i].page_size = 0;
        cache[i].title = NULL;
        cache[i].page = NULL;
        cache[i].created_time = 0;
        cache[i].client_count = 0;
        cache[i].is_complete = 0;
        cache[i].download_timeout = 0;
        cache[i].cache_timeout = 0;
        cache[i].server_fd = EMPTY;
        cache[i].server_thread = EMPTY;
        cache[i].has_server_thread = 0;
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

    pthread_t cleanup_thread;
    if (pthread_create(&cleanup_thread, NULL, cache_cleanup_thread, NULL) != 0) {
        perror("pthread_create cleanup");
        signal_handler(0);
    }

    struct pollfd fds[1];
    fds[0].fd = socket_d;
    fds[0].events = POLLIN;

    running = 1;

    while (running) {
        int ret = poll(fds, 1, TIMEOUT * 1000);
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

        if (fds[0].revents & POLLIN) {
            pthread_mutex_lock(&accept_mutex);
            while (acceptClient() == -1 && running) {
                printf("[DEBUG]: Too many clients, waiting for a free slot\n");
                pthread_cond_wait(&accept_cond, &accept_mutex);
            }
            pthread_mutex_unlock(&accept_mutex);
        }
    }

    pthread_join(cleanup_thread, NULL);
    return 0;
}