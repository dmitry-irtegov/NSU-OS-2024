#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>

#include <termios.h>
#include <pthread.h>

#define GET "GET "
#define REQ2 " HTTP/1.1\r\nHost: "  
#define REQ3 "\r\nConnection: close\r\n\r\n"
#define BUFFER_SIZE 4096

#define HEIGHT 23
#define PROMPT "Press space to scroll down"

typedef struct grar_s {
    char* str;
    size_t curlen;
    size_t maxlen;
    size_t curpos;
    char curchar;
    pthread_mutex_t mutex;
} grar_t;


struct termios oldt;
int enddata;


void print_prompt() {
    printf("\033[s");
    printf("\033[%d;1H", HEIGHT + 2);
    printf("\033[K"); 
    printf("%s", PROMPT);
    printf("\033[u");
    fflush(stdout);
}

void clear_prompt() {
    printf("\033[s");
    printf("\033[%d;1H", HEIGHT + 2);
    printf("\033[K"); 
    printf("\033[u");
    fflush(stdout);
}

void grar_init(grar_t* arr)
{
    int mutex_init_result = pthread_mutex_init(&(arr->mutex), NULL);
    arr->curlen = 0;
    arr->maxlen = BUFFER_SIZE;
    arr->curpos = 0;
    arr->curchar = 0;
    arr->str = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (arr->str == NULL) {
        perror("grar_init: calloc failed");
        exit(1);
    }
}

void grar_addbuf(grar_t* arr, char* buf, int len)
{
    pthread_mutex_lock(&(arr->mutex));

    if(arr->curlen + len >= arr->maxlen) {
        size_t new_maxlen = arr->maxlen * 2;
        char* new_str = (char*)realloc(arr->str, new_maxlen);
        if (new_str == NULL) {
            fprintf(stderr, "unable to realloc");
            pthread_mutex_unlock(&(arr->mutex));
            exit(1);
        }
        arr->str = new_str;
        arr->maxlen = new_maxlen;
    }

    memcpy(arr->str + arr->curlen, buf, len);
    arr->curlen += len;
    pthread_mutex_unlock(&(arr->mutex));
}

void grar_getstr(grar_t* arr, char** start)
{
    pthread_mutex_lock(&(arr->mutex));
    if(arr->curpos != 0) {
        arr->str[arr->curpos] = arr->curchar;
    }
    int upbord = arr->curlen - 1 > 0 ? arr->curlen - 1 : 0;
    for(int i = arr->curpos; i < upbord; i++) {
        if(arr->str[i] == '\n') {
            arr->curchar = arr->str[i + 1];
            arr->str[i + 1] = 0;
            *start = arr->str + arr->curpos;
            arr->curpos = i + 1;
            pthread_mutex_unlock(&(arr->mutex));
            return;
        }
    }

    *start = NULL;
    pthread_mutex_unlock(&(arr->mutex));
}

typedef struct url_s{
    char host[64];
    char port[8];
    char path[128];
} url_t;

url_t parse_url(const char *url) {
    url_t parsed_url = {"", "", ""};
    char url_copy[256];
    strcpy(url_copy, url);

    char *token;
    token = strtok(url_copy, "/");
    if (token != NULL) {
        char *port_ptr = strchr(token, ':');
        if (port_ptr != NULL) {
            *port_ptr = '\0';
            strcpy(parsed_url.host, token);
            strcpy(parsed_url.port, port_ptr + 1);
        } else {
            strcpy(parsed_url.host, token);
        }
    }

    token = strtok(NULL, "");
    if (token != NULL) {
        strcpy(parsed_url.path, token);
    }

    return parsed_url;
}


void* thread_printer(void* arg)
{
    struct termios newt = oldt;
    newt.c_lflag &= ~ICANON;
    newt.c_lflag &= ~ECHO;
    newt.c_cc[VMIN] = 1;
    if(tcsetattr(STDIN_FILENO, TCSADRAIN, &newt) == -1) {
        fprintf(stderr, "unable to change terminal attributes\n");
        exit(1);
    }
    
    grar_t* arr = (grar_t*)arg;
    int strcnt = HEIGHT;
    while(1) {
        if(strcnt > 0 || getchar() == ' ') {
            char* sbegin = NULL;
            grar_getstr(arr, &sbegin);
            if (sbegin != NULL) {
                clear_prompt();
                printf("%s", sbegin);
                if(strcnt > 0)
                    strcnt--;
                if(strcnt == 0) {
                    print_prompt();
                }
            } else if (enddata) {
                tcsetattr(STDIN_FILENO, TCSADRAIN, &oldt);
                pthread_exit(NULL);
            }
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        fprintf(stderr, "error: no hostname was specified\n");
        return 1;
    }

    int status, socket_fd;
    enddata = 0;
    struct addrinfo adrinf, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    char buffer[BUFFER_SIZE];

    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        fprintf(stderr, "unable to get terminal attributes\n");
        exit(1);
    }

    memset(&adrinf, 0, sizeof adrinf);
    adrinf.ai_family = AF_UNSPEC;
    adrinf.ai_socktype = SOCK_STREAM;

    url_t url = parse_url(argv[1]);
    if(strcmp(url.port, "") == 0) {
        strcpy(url.port, "80");
    }

    if ((status = getaddrinfo(url.host, url.port, &adrinf, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    for(p = res; p != NULL; p = p->ai_next) {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1) {
            perror("unable to create socket");
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd);
            perror("unable to connect");
            continue;
        }
        break;
    }

    freeaddrinfo(res);


    char *req = calloc(sizeof(GET) + strlen(url.path) + sizeof(REQ2) 
        + strlen(url.host) + sizeof(REQ3), sizeof(char));
    strcpy(req, GET);
    strcat(req, url.path);
    strcat(req, REQ2);
    strcat(req, url.host);
    strcat(req, REQ3);

    ssize_t bytes_sent = send(socket_fd, req, strlen(req), 0);
    if (bytes_sent == -1) {
        perror("unable to send");
        close(socket_fd);
        return 3;
    }

    grar_t arr;
    grar_init(&arr);
    pthread_t thread;
    int pthread_create_result = pthread_create(&thread, NULL, thread_printer, (void*)&arr);

    ssize_t bytes_received;
    while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        grar_addbuf(&arr, buffer, bytes_received);
    }
    enddata = 1;
    if (bytes_received == -1) {
        fprintf(stderr, "unable to receive bytes");
    }

    close(socket_fd);
    pthread_join(thread, NULL);
    return 0;
}