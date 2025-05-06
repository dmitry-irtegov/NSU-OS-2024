#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>    

#define BUFFER_SIZE 4096
#define HOST "127.0.0.1:6000"
int sockfd;

void error(const char* msg) {
    perror(msg);
    exit(1);
}

typedef struct data data;
typedef struct cache cache;

typedef struct data{
    int len;
    char* data;
    data* next;
}data;

typedef struct cache{
    char* request;
    int live_time;
    data* dat;
    cache* next;
}cache;

typedef struct client{
    int cli_fd;
    int inet_fd;
    char* host;
    int tot;
    int len;
    int writing_to_client;
    char buffer[4096];
    int writing;
    int writing_to_client_total;
    cache* cur_cache;
    struct client* next;
}client;


cache* head = NULL;

cache* add_to_cache(char* req, int live, char* buff, int len) {
    cache* t = head;
    while (t) {
        if (t->request && !strcmp(req, t->request)) {
            return NULL; 
        }
        t = t->next;
    }

    cache* new_cache = (cache*)malloc(sizeof(cache));
    if (!new_cache) return NULL;

    new_cache->request = strdup(req); 
    new_cache->live_time = live;
    new_cache->dat = (data*)malloc(sizeof(data));
    if (!new_cache->dat) {
        free(new_cache->request);
        free(new_cache);
        return NULL;
    }

    new_cache->dat->len = len;
    new_cache->dat->data = (char*)malloc(len);
    if (!new_cache->dat->data) {
        free(new_cache->dat);
        free(new_cache->request);
        free(new_cache);
        return NULL;
    }
    memcpy(new_cache->dat->data, buff, len); 
    new_cache->dat->next = NULL;
    new_cache->next = NULL;

    if (!head) {
        head = new_cache;
    }
    else {
        t = head;
        while (t->next) t = t->next;
        t->next = new_cache;
    }

    return new_cache;
}

void fr_data(data* d) {
    while (d) {
        data* next = d->next;
        free(d->data); 
        free(d);
        d = next;
    }
}

void remove_from_cache(cache* a) {
    if (!a) return;

    cache** p = &head;
    while (*p && *p != a) p = &(*p)->next;
    if (*p) {
        *p = a->next; 
        fr_data(a->dat);
        free(a->request);
        free(a);
    }
}

int get_content_length_from_headers(const char* headers) {
    const char* content_length_header = "Content-Length:";
    const char* ptr = strstr(headers, content_length_header);

    if (ptr == NULL) {
        return -1;  // Заголовок Content-Length не найден
    }

    // Перемещаем указатель на начало значения
    ptr += strlen(content_length_header);

    // Пропускаем пробелы
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }

    // Извлекаем числовое значение
    int length = 0;
    while (*ptr && isdigit(*ptr)) {
        length = length * 10 + (*ptr - '0');
        ptr++;
    }

    return length;
}

void parse_http_request(const char* request, char* host) {
    char method[16], path[256], version[16];

    sscanf(request, "%15s %255s %15s", method, path, version);

    const char* host_header = strstr(request, "Host:");
    if (host_header) {
        sscanf(host_header, "Host: %255s", host);
        // Удаляем возможные \r\n в конце
        char* end = host + strlen(host) - 1;
        while (end >= host && (*end == '\r' || *end == '\n')) *end-- = '\0';
    }

    // printf("Method: %s\nPath: %s\nVersion: %s\nHost: %s\n", method, path, version, host);
}

void con_to_host(int* sockfd, char* host) {
    struct hostent* server;
    struct sockaddr_in serv_addr;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
        error("Ошибка открытия сокета");

    server = gethostbyname(host);
    if (server == NULL)
        error("Ошибка: нет такого хоста");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(80);  // HTTP 

    if (connect(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Ошибка подключения");
}

void con_to_cli(int* sockfd) {
    struct hostent* server;
    struct sockaddr_in serv_addr;

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0)
        error("Ошибка открытия сокета");

    int opt = 1;
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9002);

    if (bind(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("Ошибка подключения");

}

void set_nonblocking(int cl_fd) {
    int flags = fcntl(cl_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL) failed");
        return; 
    }

    if (fcntl(cl_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL) failed");
    }
}

int add(client** head, int cl_fd) {
    client* a = malloc(sizeof(client));
    if (!a) return -1;

    a->cli_fd = cl_fd;
    set_nonblocking(cl_fd);
    a->next = NULL;
    a->tot = 0;
    a->len = 0;
    a->writing = 0;
    a->writing_to_client = 0;
    a->writing_to_client_total = 0;
    a->host = (char*)malloc(BUFFER_SIZE);
    if (!a->host) {
        free(a);
        return -1;
    }
    a->host[0] = '\0'; 
    a->inet_fd = 0;
    a->cur_cache = NULL;

    if (*head == NULL) {
        *head = a;
    }
    else {
        client* t = *head;
        while (t->next) t = t->next;
        t->next = a;
    }

    return 0;
}

client* find_cli();

int main() {
    char buffer[BUFFER_SIZE];
    char hst[1024];

    int client_socket;
    int server_socket; 
    con_to_cli(&server_socket);
    if (listen(server_socket, 5) < 0) {
        error("Ошибка listen");
    }

    client* head = NULL;

    client_socket = accept(server_socket, NULL, NULL);
    printf("hello\n");

    add(&head, client_socket);

    printf("start\n");

    while (1) {
        struct timeval tv;
        tv.tv_sec = 5;         // 5 секунд
        tv.tv_usec = 0;
        client* cur = head;
        fd_set read_fds;
        fd_set write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_SET(server_socket, &read_fds);

        //Setting fd_sets
        int max_fd = server_socket;
        while (cur) {
            FD_SET(cur->cli_fd, &read_fds);
            FD_SET(cur->cli_fd, &write_fds);
            if (cur->inet_fd > 0)
                FD_SET(cur->inet_fd, &read_fds);
            if (cur->cli_fd > max_fd) max_fd = cur->cli_fd;
            if (cur->inet_fd > max_fd) max_fd = cur->inet_fd;
            cur = cur->next;
        }
        int activity = select(max_fd + 1, &read_fds, &write_fds, NULL, &tv);
        if (activity < 0) {
            perror("select");
            exit(1);
        }


        //New connecting
        if (FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, NULL, NULL);
            if (client_socket < 0) {
                perror("accept");
                continue;
            }
            printf("New client %d\n", client_socket);
            add(&head, client_socket);
        }

        cur = head;
        while (cur) {
            client* next = cur->next;
            if (FD_ISSET(cur->cli_fd, &read_fds)) {
                
                int r = read(cur->cli_fd, buffer, BUFFER_SIZE - 1);

                //Closing client
                if (r <= 0) {
                    // Закрыть клиента
                    close(cur->cli_fd);
                    if (cur->inet_fd > 0) close(cur->inet_fd);
                    // удалить из списка
                    client** p = &head;
                    while (*p && *p != cur) p = &(*p)->next;
                    if (*p) *p = cur->next;
                    free(cur);
                    cur = next;
                    continue;
                }

                buffer[r] = '\0';

                //Write to new server
                if (!cur->writing) {
                    parse_http_request(buffer, hst);
                    if (strcmp(hst, cur->host)) {
                        con_to_host(&cur->inet_fd, hst);
                        strcpy(cur->host, hst);
                    }
                    if ((r = write(cur->inet_fd, buffer, strlen(buffer))) < 0)
                        error("Err: writing in socket");
                }

            }

            int bytes_read;

            //If don't finish writing
            if (FD_ISSET(cur->cli_fd, &write_fds) && cur->writing_to_client) {
                cur->writing_to_client += write(cur->cli_fd, cur->buffer+cur->writing_to_client, cur->writing_to_client_total - cur->writing_to_client);
                if (cur->writing_to_client == cur->writing_to_client_total) cur->writing_to_client = 0;
            }

            //Write from server to client
            if (FD_ISSET(cur->cli_fd, &write_fds) && FD_ISSET(cur->inet_fd, &read_fds) && !cur->writing_to_client && cur->tot <= cur->len &&
                (bytes_read = read(cur->inet_fd, cur->buffer, BUFFER_SIZE - 1)) > 0) {
                int l = get_content_length_from_headers(cur->buffer);
                cur->len = l == -1 ? cur->len : l;
                cur->buffer[bytes_read] = '\0';
                cur->writing_to_client = write(cur->cli_fd, cur->buffer, bytes_read);
                cur->writing_to_client_total = bytes_read;
                if (cur->writing_to_client == bytes_read) cur->writing_to_client = 0;
                cur->tot += bytes_read;
            }  

            if (bytes_read < 0)
                error("Err: reading from socket");
            cur = cur->next;
        }
        
    }

    close(client_socket);
    close(sockfd);
    return 0;
}