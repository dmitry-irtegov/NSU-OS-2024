#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <termios.h>
#include <aio.h>
#include <errno.h>

#define BUFFER_SIZE 50
#define SCREEN_LINES 5

int connectToHost(char *host, int port);
int parseUrl(const char *url, char **host, char **path, int *port);
int sendGetRequest(int sockfd, char *path, char *host);
int setNoncanonical();
int recieveGetResponse(int sockfd);
int writeRecieved(char *buffer, int size, int *printedLines);
void aiocbFilling(struct aiocb *request, int fildes, void *buff, size_t nbytes);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <URL>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *url = argv[1];
    char *host;
    char *path;
    int port = 80;

    if (parseUrl(url, &host, &path, &port) == -1) {
        exit(EXIT_FAILURE);
    }
    
    int sockfd = connectToHost(host, port);
    if (sockfd == -1) {
        exit(EXIT_FAILURE);
    }

    if (sendGetRequest(sockfd, path, host) == -1) {
        exit(EXIT_FAILURE);
    }

    if (setNoncanonical() == -1) {
        exit(EXIT_FAILURE);
    }

    if (recieveGetResponse(sockfd) == -1) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int connectToHost(char *host, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        return -1;
    }
    struct sockaddr_in serverAddr;
    struct hostent *server = gethostbyname(host);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    printf("Try to connect\n");

    if (connect(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Cannot establish connection");
        return -1;
    }
    return sockfd;
}

int parseUrl(const char* url, char** host, char** path, int* port) {
    const char* httpPrefix = "http://";
    const size_t httpPrefixLen = 7;
    
    if (strncmp(url, httpPrefix, httpPrefixLen) != 0) {
        return -1;
    }
    
    const char* hostStart = url + httpPrefixLen;
    const char* hostEnd = strchr(hostStart, '/');
    const char* portStart = strchr(hostStart, ':');
    
    if (hostEnd == NULL) {
        hostEnd = url + strlen(url);
    }
    
    if (portStart != NULL && (hostEnd == NULL || portStart < hostEnd)) {
        size_t hostLen = portStart - hostStart;
        *host = malloc(hostLen + 1);
        if (*host == NULL) return -1;
        strncpy(*host, hostStart, hostLen);
        (*host)[hostLen] = '\0';
        
        const char* portEnd = (hostEnd == NULL) ? url + strlen(url) : hostEnd;
        size_t portLen = portEnd - (portStart + 1);
        char* portStr = malloc(portLen + 1);
        if (portStr == NULL) {
            free(*host);
            return -1;
        }
        strncpy(portStr, portStart + 1, portLen);
        portStr[portLen] = '\0';
        
        *port = atoi(portStr);
        free(portStr);
    } else {
        size_t hostLen = hostEnd - hostStart;
        *host = malloc(hostLen + 1);
        if (*host == NULL) return -1;
        strncpy(*host, hostStart, hostLen);
        (*host)[hostLen] = '\0';
        *port = 80;
    }
    
    if (hostEnd != NULL) {
        size_t pathLen = strlen(hostEnd);
        *path = malloc(pathLen + 1);
        if (*path == NULL) {
            free(*host);
            return -1;
        }
        strcpy(*path, hostEnd);
    } else {
        *path = malloc(2);
        if (*path == NULL) {
            free(*host);
            return -1;
        }
        strcpy(*path, "/");
    }
    return 0;
}

int sendGetRequest(int sockfd, char *path, char *host) {
    size_t requestLen = 30 + strlen(path) + strlen(host);
    char *request = malloc(requestLen);
    if (request == NULL) {
        free(path);
        free(host);
        perror("Cannot allocate mem to request");
        return -1;
    }
    snprintf(request, requestLen, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);

    printf("request: %s\n", request);

    free(path);
    free(host);

    if (write(sockfd, request, strlen(request)) < 0) {
        free(request);
        perror("Cannot write by the socket");
        return -1;
    }
    free(request);
    return 0;
}

int setNoncanonical() {
    struct termios ttystate;
    if (tcgetattr(STDIN_FILENO, &ttystate) == -1) {
        perror("Cannot get attributes");
        return -1;
    }
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttystate) == -1) {
        perror("Cannot set noncanon");
        return -1;
    }
    return 0;
}

int recieveGetResponse(int sockfd) {
    char buffer[BUFFER_SIZE], userinputBuff[1];
    int printedLines = 0, waitForInput = 0, offset = 0, prevOffset = 0;
    struct aiocb readrq, userinput;
    const struct aiocb *asyncs[] = { &readrq, &userinput, NULL };

    memset(&readrq, 0, sizeof(readrq));
    aiocbFilling(&readrq, sockfd, buffer, BUFFER_SIZE);
    aio_read(&readrq);
    memset(&userinput, 0, sizeof(userinput));
    aiocbFilling(&userinput, STDERR_FILENO, userinputBuff, 1);
    aio_read(&userinput);

    ssize_t unwrited = 0;

    while (1) {
        aio_suspend(asyncs, 2, NULL);
        ssize_t input_size = aio_return(&userinput);
        if (input_size > 0) {
            switch (userinputBuff[0]) {
                case ' ':
                    if (waitForInput) {
                        prevOffset = offset;
                        offset = writeRecieved(buffer + offset, unwrited - offset, &printedLines);
                        if (offset > 0) {
                            unwrited -= offset - prevOffset;
                            memmove(buffer, buffer + offset, unwrited);
                            aiocbFilling(&readrq, sockfd, buffer + unwrited, BUFFER_SIZE - unwrited);
                            aio_read(&readrq);
                            offset = 0;
                        } else {
                            unwrited = 0;
                            waitForInput = 0;
                        }
                    }
                    break;
                case 'q': return 0;
            }
            aiocbFilling(&userinput, STDIN_FILENO, userinputBuff, 1);
            aio_read(&userinput);
        }

        size_t data_size = aio_return(&readrq);
        if (data_size > 0) {
            unwrited += data_size;
            if (waitForInput == 0) {
                prevOffset = offset;
                offset = writeRecieved(buffer, data_size, &printedLines);
                if (offset > 0) {
                    unwrited -= offset - prevOffset;
                    waitForInput = 1;
                } else {unwrited = 0; }
            }
            aiocbFilling(&readrq, sockfd, buffer + unwrited, BUFFER_SIZE - unwrited);
            aio_read(&readrq);
        } else if (data_size == 0 && unwrited == 0) break;
    }
    return 0;
}

int writeRecieved(char *buffer, int size, int *printedLines) {
    for (int i = 0; i < size; i++) {
        if (buffer[i] == '\n') {
            (*printedLines)++;
        }
        if (*printedLines >= SCREEN_LINES && i < size - 1) {
            write(STDOUT_FILENO, buffer, i + 1);
            printf("\nPress SPACE to scroll down or 'q' to finish.\n");
            *printedLines = 0;
            return i + 1;
        }
    }
    write(STDOUT_FILENO, buffer, size);
    return 0;
}

void aiocbFilling(struct aiocb *request, int fildes, void *buff, size_t nbytes) {
    request->aio_fildes = fildes;
    request->aio_buf = buff;
    request->aio_nbytes = nbytes;
}
