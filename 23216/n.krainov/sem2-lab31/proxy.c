#include "proxy.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

extern ProxyState proxy;

int registerRequest(int fd) {
    int newConn = accept(fd, NULL, NULL);
    if (newConn == -1) return 1;
    
    if (fcntl(newConn, F_SETFL, O_NONBLOCK) == -1) {
        return 1;
    }

    if (proxy.countPFDs == proxy.lenPFDs) {
        proxy.pfds = realloc(proxy.pfds, proxy.lenPFDs * 2);
        if (proxy.pfds == NULL) {
            return 1;
        }

        proxy.lenPFDs = proxy.lenPFDs * 2;
        proxy.pfds[proxy.countPFDs].fd = newConn;
        proxy.pfds[proxy.countPFDs].events = POLLIN;
        proxy.types[proxy.countPFDs] = REQUEST;

    } else {
        for (int i = 1; i < proxy.lenPFDs; i++) {
            if (proxy.pfds[i].fd == -1) {
                proxy.pfds[i].fd = newConn;
                proxy.pfds[i].events = POLLIN;
                proxy.types[i] = REQUEST;
                break;
            }
        }
    }

    for (int i = 0; i < proxy.lenRequests; i++) {
        if (proxy.requests[i].fd == -1) {
            proxy.requests[i].fd = newConn;
            free(proxy.requests[i].req);
            proxy.requests[i].req = calloc(1, sizeof(Buffer));
            if (proxy.requests[i].req == NULL) {
                return 1;
            }

            proxy.requests[i].req->count = 0;
            proxy.requests[i].req->len = 512;
            proxy.requests[i].req->buffer = calloc(512, sizeof(char));

            if (proxy.requests[i].req->buffer == NULL) {
                return 1;
            }

            return 0;
        }
    }

    return 1;
}

int registerConnect(Loader loader) {
    struct sockaddr_in servaddr;
    char* target = strstr(loader.request->buffer, "Host:");

    if (target == NULL) {
        return -1;
    }
    
    target += 5;
    while (isspace(*target)) {
        target++;
    }

    char* start = target;
    int len = 0;

    while (!isspace(*target)) {
        target++;
        len++;
    }

    char* res = calloc(len + 1, sizeof(char));

    if (res == NULL) {
        return -1;
    }

    strncpy(res, start, len);

    struct hostent* hosts = gethostbyname(res);

    if (hosts == NULL) {
        return -1;
    }

    free(res);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        return -1;
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(HTTP_PORT);
    
    memcpy(&servaddr.sin_addr.s_addr, hosts->h_addr_list[0], hosts->h_length);

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        return -1;
    } 

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) && errno != EINPROGRESS) {
        return -1;
    }

    errno = 0;

    if (proxy.countPFDs == proxy.lenPFDs) {
        proxy.pfds = realloc(proxy.pfds, proxy.lenPFDs * 2);

        if (proxy.pfds == NULL) {
            return -1;
        }

        proxy.lenPFDs = proxy.lenPFDs * 2;
        proxy.pfds[proxy.countPFDs].fd = sockfd;
        proxy.pfds[proxy.countPFDs].events = POLLOUT;
        proxy.types[proxy.countPFDs] = CONNECTING;
    } else {
        for (int i = 1; i < proxy.lenPFDs; i++) {
            if (proxy.pfds[i].fd == -1) {
                proxy.pfds[i].fd = sockfd;
                proxy.pfds[i].events = POLLOUT;
                proxy.types[i] = CONNECTING;
                break;
            }
        }
    }

    return sockfd;
}

int checkEndOfReq(Buffer* req) {
    return req->buffer[req->count-1] == '\n' && req->buffer[req->count-2] == '\r' 
        && req->buffer[req->count-3] == '\n' && req->buffer[req->count-4] == '\r';
     
}

int analyzeRequest(int indexReq) {
    Buffer* help = calloc(1, sizeof(Buffer));

    if (help == NULL) {
        return 1;
    }

    for (int i = 1; i < proxy.requests[indexReq].req->count; i++) {
        if (proxy.requests[indexReq].req->buffer[i] == '\n' && proxy.requests[indexReq].req->buffer[i - 1] == '\r') {
            help->count = i + 1;
            help->len = i + 2;
            help->buffer = calloc(help->len, sizeof(char));
            if (help->buffer == NULL) {
                return 1;
            }

            strncpy(help->buffer, proxy.requests[indexReq].req->buffer, help->count);
            break;
        }
    }

    char* check1 = strstr(help->buffer, "HEAD");
    char* check2 = strstr(help->buffer, "GET");
    char* check3 = strstr(help->buffer, "HTTP/1.0");

    if (!check1 && !check2 && !check3) {
        //послать клиенту ошибку

        return 2;
    }

    proxy.requests[indexReq].keyCache = help;

    return 0;
}

int initLoader(int indexReq) {
    for (int i = 0; i < proxy.lenLoaders; i++) {
        if (proxy.loaders[i].fd == -1) {
            proxy.loaders[i].key = proxy.requests[i].keyCache;
            proxy.loaders[i].request = proxy.requests[i].req;
            proxy.loaders[i].fd = registerConnect(proxy.loaders[i]);

            if (proxy.loaders[i].fd == -1) {
                return 1;
            }

            return 0;
        }
    }

    //сделаю потом (расширение loaders)
}

int checkCache(int index) {
    CacheEntry* entry;
    if ((entry = getPage(proxy.requests[index].keyCache)) == NULL) {
        if (initLoader(index)) {
            return 1;
        }
    } 

    return 0;
}

int processRequest(int index) {
    for (int i = 0; i < proxy.lenRequests; i++) {
        if (proxy.requests[i].fd == proxy.pfds[index].fd) {
            int cnt = 0;
            while ((cnt = read(proxy.requests[i].fd, 
                               proxy.requests[i].req->buffer + proxy.requests[i].req->count, 
                               proxy.requests[i].req->len - proxy.requests[i].req->count))) {
                
                proxy.requests[i].req->count += cnt;

                if (checkEndOfReq(proxy.requests[i].req)) {
                    if (analyzeRequest(i) == 1) {
                        return 1;
                    }

                    if (checkCache(i)) {
                        return 1;
                    }

                    Buffer* buf = calloc(1, sizeof(Buffer));

                    if (buf == NULL) {
                        return 1;
                    }

                    buf->buffer = calloc(2048, sizeof(char));

                    if (buf->buffer == NULL) {
                        return 1;
                    }

                    printf("request\n%s\n", proxy.loaders[i].request->buffer);
                    buf->len = 2048;
                    buf->count = 0;
                    putInCache(proxy.requests[i].keyCache, buf);
                    proxy.pfds[index].events = POLLOUT;
                    proxy.types[index] = ANSWER;
                    return 0;
                }
                else if (proxy.requests[i].req->count == proxy.requests[i].req->len) {
                    proxy.requests[i].req->buffer = realloc(proxy.requests[i].req->buffer, proxy.requests[i].req->len * 2);
                    proxy.requests[i].req->len *= 2;

                    if (proxy.requests[i].req->buffer == NULL) {
                        return 1;
                    }
                }
            }
            break;
        }
    }
}

int processConnecting(int index) {
    int err;
    socklen_t sz = sizeof(err);
    if (getsockopt(proxy.pfds[index].fd, SOL_SOCKET, SO_ERROR, &err, &sz) != 0) {
        return 1;
    }

    if (err != 0) {
        return 1;
    }

    proxy.types[index] = SENDING_REQ;
    return 0;
}

int processSendingReq(int index) {
    for (int i = 0; i < proxy.lenLoaders; i++) {
        if (proxy.loaders[i].fd == proxy.pfds[index].fd) {
            int cnt = 0;
            if ((cnt = write(proxy.loaders[i].fd, proxy.loaders[i].request->buffer + proxy.loaders[i].curIndex, proxy.loaders[i].request->count)) == -1) {
                return 1;
            }

            proxy.loaders[i].curIndex += cnt;

            if (proxy.loaders[i].curIndex == proxy.loaders[i].request->count) {
                proxy.types[index] = SERVER_ANSWER;
                proxy.pfds[index].events = POLLIN;  
                proxy.loaders[i].curIndex = 0;
            } 
            
            return 0;
        }
    }
}

int checkEndOfServerAnswer() {
    return 1;
}

int processServerAnswer(int index) {
    for (int i = 0; i < proxy.lenLoaders; i++) {
        if (proxy.loaders[i].fd == proxy.pfds[index].fd) {
            CacheEntry* entry = getPage(proxy.loaders[i].key);
            ssize_t count;

            if (entry == NULL || entry->status != 0) {
                return 1;
            }

            entry->val->count = 0;
            while ((count = read(proxy.loaders[i].fd, entry->val->buffer + entry->val->count, entry->val->len))) {
                entry->val->count += count;
                
                if (entry->val->count == entry->val->len) {
                    entry->val->len *= 2;
                    entry->val->buffer = realloc(entry->val->buffer, entry->val->len);
                }
                count = 0;
            }

            if (checkEndOfServerAnswer()) {
                entry->status = 1;
                close(proxy.pfds[index].fd);
                proxy.pfds[index].fd = -1;
                proxy.loaders[i].fd = -1;
                proxy.loaders[i].key = NULL;
                proxy.loaders[i].request = NULL;
                proxy.loaders[i].curIndex = 0;
            } 
            
            return 0;
        }
    }
}

int processAnswer(int index) {
    for (int i = 0; i < proxy.lenLoaders; i++) {
        if (proxy.requests[i].fd == proxy.pfds[index].fd) {
            CacheEntry* entry = getPage(proxy.requests[i].keyCache);

            if (entry->status == 1) {
                ssize_t cnt = write(proxy.requests[i].fd, entry->val->buffer + proxy.requests[i].curIndex, entry->val->count - proxy.requests[i].curIndex);
                if (cnt == -1) {
                    exit(1);
                    return 1;
                }

                proxy.requests[i].curIndex += cnt;

                if (proxy.requests[i].curIndex == entry->val->count) {
                    close(proxy.requests[i].fd);
                    proxy.pfds[index].fd = -1;
                    proxy.requests[i].curIndex = 0;
                    proxy.requests[i].fd = -1;
                    proxy.requests[i].keyCache = NULL;
                    proxy.requests[i].req = NULL;
                } 
                
                
            }




            return 0;
        }
    }
}

int processInputData(int index) {
    int retval = -1;
    switch (proxy.types[index]) {
        case REQUEST:
            retval = processRequest(index);
            break;
        case SERVER_ANSWER:
            retval = processServerAnswer(index);
            break;
    } 

    return retval;
}

int processOutputData(int index) {
    int retval = -1;
    switch (proxy.types[index]) {
        case CONNECTING:
            retval = processConnecting(index);
            break;
        case SENDING_REQ:
            retval = processSendingReq(index);
            break;
        case ANSWER:
            retval = processAnswer(index);
            break;
    } 

    return retval;
}

int workLoop() {
    while (1) {
        int count;
        switch ((count = poll(proxy.pfds, proxy.lenPFDs, -1))) {
            case -1:
                return 1;
            case 0:
                break;
            default:
                if (proxy.pfds[0].revents & POLLIN) {
                    proxy.pfds[0].revents = 0;
                    if (registerRequest(proxy.pfds[0].fd)) {
                        return 1;
                    }
                    count--;
                }

                for (int i = 1; i < proxy.lenPFDs && count != 0; i++) {
                    if (proxy.pfds[i].revents & POLLIN) {
                        processInputData(i);
                        count--;
                    } else if (proxy.pfds[i].revents & POLLOUT) {
                        processOutputData(i);
                        count--;
                    }

                    proxy.pfds[i].revents = 0;
                }
                
                
        }
    }
}