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

int registerRequest(int fd) {
    int newConn = accept(fd, NULL, NULL);
    if (newConn == -1) {
        return 1;
    }

    if (fcntl(newConn, F_SETFL, O_NONBLOCK) == -1) {
        close(newConn);
        return -1;
    }

    if (addToPFDs(newConn, POLLIN, REQUEST)) {
        close(newConn);
        return 1;
    }

    if (addToRequests(newConn)) {
        removeFromPFDs(newConn);
        close(newConn);
        return 1;
    }

    return 0;
}

int registerConnect(Buffer* url) {
    struct sockaddr_in servaddr;

    int end = url->count;
    for (int i = 0; i < url->count; i++) {
        if (url->buffer[i] == '/') {
            end = i;
            break;
        }
    }

    Buffer* host = initBuffer(end, url->buffer);

    struct hostent* hosts = gethostbyname(host->buffer);
    
    freeBuffer(host);
    
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

int initLoader(Request* req) {
    int startIndex = 0;
    if (proxy.lenLoaders == proxy.countLoaders) {
        proxy.loaders = realloc(proxy.loaders, sizeof(Loader) * proxy.lenLoaders * 2);
        if (proxy.loaders == NULL) {
            return 1;
        }

        proxy.lenLoaders *= 2;
        startIndex = proxy.countLoaders;
        for (int i = startIndex; i < proxy.lenLoaders; i++) {
            proxy.loaders[i].fd = -1;
        }
    }
    
    for (int i = startIndex; i < proxy.lenLoaders; i++) {
        if (proxy.loaders[i].fd == -1) {
            Buffer* buf = initBuffer(2048, NULL); 
            if (buf == NULL) {
                return 1;
            }

            proxy.loaders[i].request = initBuffer(req->req->count, req->req->buffer);
            if (proxy.loaders[i].request == NULL) {
                freeBuffer(buf);
                return 1;
            }

            Buffer* Lkey = initBuffer(req->keyCache->count, req->keyCache->buffer);
            if (Lkey == NULL) {
                freeBuffer(buf);
                return 1;
            }

            proxy.loaders[i].key = Lkey;

            proxy.loaders[i].fd = registerConnect(req->keyCache);
            if (proxy.loaders[i].fd == -1) {
                free(proxy.loaders[i].request);
                freeBuffer(buf);
                return 1;
            } else if (proxy.loaders[i].fd == -2) {
                free(proxy.loaders[i].request);
                proxy.loaders[i].fd = -1;
                free(buf->buffer);

                if (status502(buf)) {
                    return 1;
                }

                if (putInCache(Lkey, buf, PAGE_ERROR)) {
                    freeBuffer(buf);
                }
                return 2;
            }

            Buffer* key = initBuffer(req->keyCache->count, req->keyCache->buffer);
            if (key == NULL) {
                freeBuffer(Lkey);
                freeBuffer(proxy.loaders[i].request);
                freeBuffer(buf);
            }

            if (putInCache(key, buf, PAGE_CHECKING)) {
                freeBuffer(buf);
                freeBuffer(Lkey);
                freeBuffer(key);
                freeBuffer(proxy.loaders[i].request);
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
    } else {
        entry->inUse++;
    }

    return 0;
}

int processRequest(int index) {
    Request* req = findRequest(index);
    
    ssize_t cnt = 0;
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
        
        if (retval == -1) { //что делать, если нет памяти?
            close(proxy.pfds[index].fd);
            removeFromPFDs(proxy.pfds[index].fd);
            removeFromRequests(proxy.pfds[index].fd);
            return 1;
        } else if (retval == 1) { //надо отправлять ошибку вообще
            close(proxy.pfds[index].fd);
            removeFromPFDs(proxy.pfds[index].fd);
            removeFromRequests(proxy.pfds[index].fd);
        
            return 1;
        }

        if (checkCache(req)) {
            return 1;
        }

        req->curIndex = 0;
        proxy.pfds[index].events = POLLOUT;
        proxy.types[index] = ANSWER;
    } else {
        removeFromPFDs(req->fd);
        removeFromRequests(req->fd);
    }


    return 0;
}

int processConnecting(int index) {
    int err;
    socklen_t sz = sizeof(err);
    if (getsockopt(proxy.pfds[index].fd, SOL_SOCKET, SO_ERROR, &err, &sz) != 0) {
        Loader* loader = findLoader(index);
        freeBuffer(loader->key);
        freeBuffer(loader->request);
        removeFromPFDs(loader->fd);
        close(loader->fd);
        
        CacheEntry* entry = getPage(loader->key);
        freeBuffer(entry->val);
        entry->status = PAGE_ERROR;
        entry->inUse--;

        if (status502(entry->val)) {
            entry->val->buffer = NULL;
            return 1;
        }
        return 1;
    }

    if (err != 0) {
        Loader* loader = findLoader(index);
        freeBuffer(loader->key);
        freeBuffer(loader->request);
        removeFromPFDs(loader->fd);
        close(loader->fd);
        
        CacheEntry* entry = getPage(loader->key);
        freeBuffer(entry->val);
        entry->status = PAGE_ERROR;
        entry->inUse--;

        if (status502(entry->val)) {
            entry->val->buffer = NULL;
            return 1;
        }
            
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
                CacheEntry* entry = getPage(loader->key);
                entry->status = PAGE_ERROR;
                entry->inUse--;
                freeBuffer(entry->val);
                freeBuffer(loader->key);
                freeBuffer(loader->request);
    
                if (status502(entry->val)) {
                    entry->val->buffer = NULL;
                    return 1;
                }
                
                removeFromPFDs(loader->fd);
                loader->fd = -1;
                
                return 1;
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
    char* protocol1 = strstr(buf->buffer, "HTTP/1.1");

    if (protocol == NULL && protocol1 == NULL) {
        return -1;
    }

    char* check = protocol == NULL ? protocol1 : protocol;

    check += strlen("HTTP/1.0");

    int status = atoi(check);

    if (!(status < 300 && status >= 200)) {
        return 1;
    }

    return 0;
}

int processServerAnswer(int index) {
    Loader* loader = findLoader(index);
    
    
    CacheEntry* entry = getPage(loader->key);
    ssize_t count;

    while ((count = read(loader->fd, entry->val->buffer + entry->val->count, entry->val->len - entry->val->count))) {
        if (count == -1) {
            if (errno == EAGAIN) {
                return 0;
            }

            if (errno == ECONNRESET) {
                CacheEntry* entry = getPage(loader->key);
                entry->status = PAGE_ERROR;
                entry->inUse--;
                freeBuffer(entry->val);
                freeBuffer(loader->key);
                freeBuffer(loader->request);
    
                if (status502(entry->val)) {
                    entry->val->buffer = NULL;
                    return 1;
                }
                
                removeFromPFDs(loader->fd);
                loader->fd = -1;
                
                return 1;
            }

            return -1;
        }

        entry->val->count += count;
        
        resizeBuffer(entry->val);

        if (entry->status == PAGE_CHECKING) {
            int ret = checkAnswer(entry->val);
            switch (ret) {
                case -1:
                    break;
                case 1:
                    entry->status = PAGE_ERROR_LOADING;
                    break;
                case 0:
                    entry->status = PAGE_SUCCESS_LOADING;
                    break;
            }
        }
    }

    
    close(proxy.pfds[index].fd);
    removeFromPFDs(loader->fd);
    loader->fd = -1;
    freeBuffer(loader->key);
    freeBuffer(loader->request);
    loader->curIndex = 0;
    switch (entry->status) {
        case PAGE_ERROR_LOADING:
            entry->status = PAGE_ERROR;
            break;
        case PAGE_SUCCESS_LOADING:
            entry->status = PAGE_LOADED;
            break;
        case PAGE_CHECKING:
            entry->status = PAGE_ERROR;
            break;
    }
    entry->inUse--;

    return 0;
}

int processAnswer(int index) {
    Request* req = findRequest(index);
    
    CacheEntry* entry = getPage(req->keyCache);
    

    if (entry->status == PAGE_SUCCESS_LOADING || entry->status == PAGE_ERROR_LOADING 
        || entry->status == PAGE_LOADED || entry->status == PAGE_ERROR) {
        ssize_t cnt;
        while ((cnt = write(req->fd, entry->val->buffer + req->curIndex, entry->val->count - req->curIndex))) {
            if (cnt == -1) {
                if (errno == EAGAIN) {
                    return 0;
                }

                if (errno == ECONNRESET) {
                    close(req->fd); 
                    entry->inUse--;
        
                    removeFromPFDs(req->fd);
                    removeFromRequests(req->fd);
                }

                return -1;
            }
        
            req->curIndex += cnt;
        }

        if (req->curIndex == entry->val->count && (entry->status == PAGE_LOADED || entry->status == PAGE_ERROR)) {
            close(req->fd); 
            entry->inUse--;
            if (entry->status == PAGE_ERROR && entry->inUse == 0) {
                removeFromCache(req->keyCache);
            } else {
                entry->timeCreating = time(NULL);
            }

            removeFromPFDs(req->fd);
            removeFromRequests(req->fd);

            
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

    int timeout = TIMEOUT_PURGE;
    while (1) {
        int count;
        switch ((count = poll(proxy.pfds, proxy.lenPFDs, timeout))) {
            case -1:
                if (!(errno == EINTR && proxy.endOfWork)) {
                    return -1;   
                }
            case 0:
                break;
            default:
                if (proxy.endOfWork == 0 && proxy.pfds[0].revents & POLLIN) {
                    proxy.pfds[0].revents = 0;
                    if (registerRequest(proxy.pfds[0].fd) == -1) {
                        return -1;
                    }  
                    count--;
                }

                for (int i = 1; i < proxy.lenPFDs && count != 0; i++) {
                    if (proxy.pfds[i].revents & POLLIN) {
                        if (processInputData(i) == -1) {
                            return -1;
                        }
                        count--;
                    } else if (proxy.pfds[i].revents & POLLOUT) {
                        if (processOutputData(i) == -1) {
                            return -1;
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
            return -1;
        }

        double diff = difftime(now, startCycle);
        if (timeout/1000 > diff) {
            timeout -= diff*1000;
        } else {
            purgeCache(now);
            timeout = TIMEOUT_PURGE;
            startCycle = now;
        }

        for (unsigned int i = 0; i < proxy.cache.cap; i++) {
            if (proxy.cache.state[i] == 1 && proxy.cache.buffers[i].status == PAGE_ERROR && proxy.cache.buffers[i].inUse == 0) {
                removeFromCache(proxy.cache.buffers[i].key);
            }
        }
    }
}