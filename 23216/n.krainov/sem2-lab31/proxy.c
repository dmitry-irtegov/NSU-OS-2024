#include "proxy.h"
#include "util.h"
#include "statuses.h"

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
#include <stdio.h>
#include <time.h>

extern ProxyState proxy;

//вообще надо бы принтовать, что происходит (кто постучался к нам, куда он пошел и так далее)
//может раскидать разные этапы по разным файлам? Сейчас это крайне трудночитабельно
//надо подумать, когда надо умереть, а когда просто кидать ошибку. Вроде при неуспехе маллока можно почистить кэш. Но тогда в чем смысл?

int registerRequest(int fd) {
    int newConn = accept(fd, NULL, NULL);
    if (newConn == -1) {
        perror("accept failed");
        return 2;
    }

    if (fcntl(newConn, F_SETFL, O_NONBLOCK) == -1) {
        return 1;
    }

    if (addToPFDs(newConn, POLLIN, REQUEST)) {
        return 1;
    }

    if (addToRequests(newConn)) {
        return 1;
    }

    return 0;
}

int registerConnect(Loader* loader) {
    struct sockaddr_in servaddr;

    struct hostent* hosts = gethostbyname(loader->key->buffer); 
    
    if (hosts == NULL) {
        return -2;
    } 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        return -1;
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);
    
    memcpy(&servaddr.sin_addr.s_addr, hosts->h_addr_list[0], hosts->h_length);

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        return -1;
    } 

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) && errno != EINPROGRESS) {
        return -1;
    }
    
    errno = 0;

    if (addToPFDs(sockfd, POLLOUT, CONNECTING)) {
        return -1;
    }

    return sockfd;
}

int analyzeRequest(Request* req) {
    char* check1 = strstr(req->req->buffer, "HEAD");
    char* check2 = strstr(req->req->buffer, "GET");
    char* check3 = strstr(req->req->buffer, "HTTP/1.0");

    if ((check1 == NULL) == (check2 == NULL) || !check3 || ((check1 == NULL) ? check2 : check1) > check3) { 
        return 1;
    }

    char* http = strstr(req->req->buffer, "http://");
    char* HTTP = strstr(req->req->buffer, "HTTP://");

    char* url = (http == NULL ? HTTP : http);
    if ((http == NULL) == (HTTP == NULL) && url < check3 &&
        url > ((check1 == NULL) ? check2 : check1)) {

        return 1;
    }

    url += strlen("HTTP://");

    char* ptr = url;
    int cap = 0;

    while (!isspace(*ptr) && *ptr != '/') {
        cap++;
        ptr++;
    }
    
    if (cap == 0) {
        return 1;
    }

    Buffer* key = initBuffer(cap, url);
    if (key == NULL) {
        return -1;
    }
    
    req->keyCache = key;

    return 0;
}

int initLoader(Request* req) {
    if (proxy.lenLoaders == proxy.countLoaders) {
        proxy.loaders = realloc(proxy.loaders, proxy.lenLoaders * 2);
        if (proxy.loaders == NULL) {
            return 1;
        }
    }
    
    for (int i = 0; i < proxy.lenLoaders; i++) {
        if (proxy.loaders[i].fd == -1) {
            Buffer* Lkey = initBuffer(req->keyCache->count, req->keyCache->buffer);
            proxy.loaders[i].key = Lkey;
            proxy.loaders[i].request = req->req;
            
            Buffer* buf = initBuffer(2048, NULL); 
            if (buf == NULL) {
                return 1;
            }

            proxy.loaders[i].fd = registerConnect(&proxy.loaders[i]);

            if (proxy.loaders[i].fd == -1) {
                return 1;
            } else if (proxy.loaders[i].fd == -2) {
                proxy.loaders[i].fd = -1;
                free(buf->buffer);
                if (status502(buf)) {
                    return 1;
                }

                putInCache(req->keyCache, buf, 1);
                return 2;
            }

            Buffer* key = initBuffer(req->keyCache->count, req->keyCache->buffer);
            if (putInCache(key, buf, 0)) {
                return 1;
            } 

            proxy.countLoaders++;

            return 0;
        }
    }

    //not reachable
    return 1;
}

int checkCache(Request* req) {
    CacheEntry* entry;
    if ((entry = getPage(req->keyCache)) == NULL) {
        if (initLoader(req) == 1) {
            return 1;
        }
    } 

    return 0;
}

int processRequest(int index) {
    Request* req = findRequest(index);
    
    int cnt = 0;
    while ((cnt = read(req->fd, 
                        req->req->buffer + req->req->count, 
                        req->req->len - req->req->count))) {
        
        if (cnt == -1) {
            if (errno == EAGAIN) {
                break;
            }

            return 1;
        }

        req->req->count += cnt;
        resizeBuffer(req->req);
    }

    if (checkEndOfReq(req->req)) {
        int retval = analyzeRequest(req);
        if (retval == -1) {
            return 1;
        } else if (retval == 1) {
            proxy.pfds[index].fd = -1;
            req->fd = -1;
            freeBuffer(req->req);
            proxy.countRequests--;
            proxy.countPFDs--;
            return 0;
        }

        if (checkCache(req)) {
            return 1;
        }

        proxy.pfds[index].events = POLLOUT;
        proxy.types[index] = ANSWER;
    } else {
        req->fd = -1;
        freeBuffer(req->req);
        proxy.countRequests--;
        proxy.pfds[index].fd = -1;
    }


    return 0;
}

int processConnecting(int index) {
    int err;
    socklen_t sz = sizeof(err);
    if (getsockopt(proxy.pfds[index].fd, SOL_SOCKET, SO_ERROR, &err, &sz) != 0) {
        return 1;
    }

    if (err != 0) {
        Loader* loader = findLoader(index);

        loader->fd = -1;
        CacheEntry* entry = getPage(loader->key);
        freeBuffer(entry->val);

        if (status502(entry->val)) {
            return 1;
        }
        
        entry->status = 1;
        proxy.pfds[index].fd = -1;    
        return 0;
    }

    proxy.types[index] = SENDING_REQ; 
    return 0;
}

int processSendingReq(int index) {
    Loader* loader = findLoader(index);
    
    ssize_t cnt = 0;
    while ((cnt = write(loader->fd, loader->request->buffer + loader->curIndex, loader->request->count - loader->curIndex))) {
        if (cnt == -1) {
            if (errno == EAGAIN) {
                errno = 0;
                return 0;
            }

            if (errno == ECONNRESET) {
                loader->fd = -1;
                CacheEntry* entry = getPage(loader->key);
                freeBuffer(entry->val);
    
                if (status502(entry->val)) {
                    return 1;
                }
                
                entry->status = 1;
                errno = 0;
                proxy.pfds[index].fd = -1;
                
                return 0;
            }

            return 1;
        }
        

        loader->curIndex += cnt;
    }

    if (loader->curIndex == loader->request->count) {
        proxy.types[index] = SERVER_ANSWER;
        proxy.pfds[index].events = POLLIN;  
        loader->curIndex = 0;
    } 
    
    return 0;
}

int checkAnswer(Buffer* buf) {
    char* protocol = strstr(buf->buffer, "HTTP/1.0");

    if (protocol == NULL) {
        return -1;
    }

    protocol+=strlen("HTTP/1.0");

    int status = atoi(protocol);

    if (!(status < 300 && status >= 200)) {
        return 1;
    }

    return 0;
}

int processServerAnswer(int index) {
    Loader* loader = findLoader(index);
    
    
    CacheEntry* entry = getPage(loader->key);
    entry->inUse = 1;
    ssize_t count;

    while ((count = read(loader->fd, entry->val->buffer + entry->val->count, entry->val->len - entry->val->count))) {
        if (count == -1) {
            if (errno == EAGAIN) {
                return 0;
            }

            return 1;
        }

        entry->val->count += count;
        
        resizeBuffer(entry->val);

        if (entry->status == 0) {
            if (checkAnswer(entry->val)) {
                entry->status = 2;
            } else {
                entry->status = 3;
            }
        }
    }

    entry->status = entry->status == 3 ? 1 : 2;
    close(proxy.pfds[index].fd);
    proxy.pfds[index].fd = -1;
    proxy.countPFDs--;
    loader->fd = -1;
    loader->key = NULL;
    loader->request = NULL;
    loader->curIndex = 0;

    return 0;
}

int processAnswer(int index) {
    Request* req = findRequest(index);
    
    CacheEntry* entry = getPage(req->keyCache);
    entry->inUse = 1;

    if (entry->status == 1 || entry->status == 2) {
        ssize_t cnt;
        while ((cnt = write(req->fd, entry->val->buffer + req->curIndex, entry->val->count - req->curIndex))) {
            if (cnt == -1) {
                if (errno == EAGAIN) {
                    return 0;
                }
                return 1;
            }
        
            req->curIndex += cnt;
        }

        if (req->curIndex == entry->val->count) {
            close(req->fd); 
            proxy.countPFDs--;
            proxy.pfds[index].fd = -1;
            req->curIndex = 0;
            req->fd = -1;
            freeBuffer(req->keyCache);
            req->keyCache = NULL;
            req->req = NULL;

            proxy.countRequests--;

            entry->inUse = 0;
            if (entry->status == 2) {
                freeBuffer(entry->val);
                freeBuffer(entry->key);
                entry->key = NULL;
                entry->val = NULL;
            }
        } 
        
        
    }




    return 0;
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
    time_t startCycle = time(NULL);
    if (startCycle == -1) {
        return 1;
    }

    int timeout = 300000;
    while (1) {
        int count;
        switch ((count = poll(proxy.pfds, proxy.lenPFDs, timeout))) {
            case -1:
                if (!(errno == EINTR && proxy.endOfWork)) {
                    return 1;   
                }
            case 0:
                break;
            default:
                if (proxy.endOfWork == 0 && proxy.pfds[0].revents & POLLIN) {
                    proxy.pfds[0].revents = 0;
                    if (registerRequest(proxy.pfds[0].fd) == 1) {
                        return 1;
                    }  
                    count--;
                }

                for (int i = 1; i < proxy.lenPFDs && count != 0; i++) {
                    if (proxy.pfds[i].revents & POLLIN) {
                        if (processInputData(i)) {
                            return 1;
                        }
                        count--;
                    } else if (proxy.pfds[i].revents & POLLOUT) {
                        if (processOutputData(i)) {
                            return 1;
                        }
                        count--;
                    }

                    proxy.pfds[i].revents = 0;
                }
        }

        if (proxy.countPFDs == 1 && proxy.endOfWork) {
            return 0;
        }

        time_t now = time(NULL);
        if (now == -1) {
            return 1;
        }
        double diff = difftime(now, startCycle);
        if (timeout/1000 > diff) {
            timeout -= diff*1000;
        } else {
            purgeCache(now);
            timeout = 300000;
            startCycle = now;
        }

        for (unsigned int i = 0; i < proxy.cache.cap; i++) {
            proxy.cache.buffers[i].inUse = 0;
        }
    }
}