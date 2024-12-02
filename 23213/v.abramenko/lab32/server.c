#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <aio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/siginfo.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024

typedef struct request_s {
    int completed;
    struct aiocb* req;
}request;

char* socket_path = "./socket";

void close_handler(int sig) {
	unlink(socket_path);
	_exit(0);
}

sigjmp_buf toprocess;

void sigiohandler(int signo, siginfo_t* info, void* context){
    if (signo != SIGIO || info->si_signo != SIGIO){
        return;
    }
    int a = 0;
    write(1, &a, sizeof(int));
    request* req = (request*)info->si_value.sival_ptr;
    if (aio_error(req->req) == 0) {
        req->completed = 1;
        siglongjmp(toprocess, 1);
    }
}

request* create_request(int cl) {
    request* request = malloc(sizeof(request));
    if (request == NULL) {
        perror("malloc failed");
        close(cl);
        return NULL;
    }

    request->req = malloc(sizeof(struct aiocb));
    if (request->req == NULL) {
        perror("malloc failed");
        free(request);
        close(cl);
        return NULL;
    }

    request->req->aio_fildes = cl;
    request->req->aio_offset = 0;
    request->req->aio_buf = malloc(BUF_SIZE * sizeof(char));
    if (request->req->aio_buf == NULL) {
        perror("malloc failed");
        free(request->req);
        free(request);
        close(cl);
        return NULL;
    }

    request->req->aio_nbytes = BUF_SIZE - 1;
    request->req->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    request->req->aio_sigevent.sigev_signo = SIGIO;
    request->req->aio_sigevent.sigev_value.sival_ptr = request;

    if (aio_read(request->req) == -1) {
        perror("aio_read failed");
        char* buf = (char*)request->req->aio_buf;
        free(buf);
        free(request->req);
        free(request);
        close(cl);
        return NULL;
    }

    return request;
}


int main() {
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("binding failed");
        close(fd);
        exit(-1);
    }

    if (listen(fd, MAX_CLIENTS) == -1) {
        perror("listen failed");
        unlink(socket_path);
        close(fd);
        exit(-1);
    }

    signal(SIGINT, close_handler);

    struct sigaction sigiohandleraction;
    sigemptyset(&sigiohandleraction.sa_mask);
    sigaddset(&sigiohandleraction.sa_mask, SIGIO);

    sigiohandleraction.sa_sigaction = sigiohandler;
    sigiohandleraction.sa_flags = SA_SIGINFO;
    sigaction(SIGIO, &sigiohandleraction, NULL);

    int cl;
    int cnt = 0;
    request* requests[MAX_CLIENTS];
    memset(requests, 0, sizeof(requests));
    while (1) {
        sigprocmask(SIG_BLOCK, &sigiohandleraction.sa_mask, NULL);
        if (sigsetjmp(toprocess, 1) != 0) {
            for (int i = 0; i < cnt; i++)
            {
                if (!requests[i]->completed) {
                    continue;
                }
                
                int rc = aio_return(requests[i]->req);
                if (rc <= 0) {
                    if (rc == -1) {
                        perror("return failed");
                    }
                    char* buf = (char*)requests[i]->req->aio_buf;
                    free(buf);
                    close(requests[i]->req->aio_fildes);
                    free(requests[i]->req);
                    free(requests[i]);
                    requests[i] = requests[cnt - 1];
                    requests[cnt - 1] = NULL; 
                    cnt--;
                    i--;
                } else {
                    char* buf = (char*)requests[i]->req->aio_buf;
                    buf[rc] = 0;
                    for (int j = 0; j < rc; j++) {
                        putchar(toupper((unsigned char)buf[j]));
                    }
                    requests[i]->completed = 0;
                    if (aio_read(requests[i]->req) == -1) {
                        free(buf);
                        close(requests[i]->req->aio_fildes);
                        free(requests[i]->req);
                        free(requests[i]);
                        requests[i] = requests[cnt - 1];
                        requests[cnt - 1] = NULL; 
                        cnt--;
                        i--;
                    }
                }
            }
        }
        sigprocmask(SIG_UNBLOCK, &sigiohandleraction.sa_mask, NULL);

        if ((cl = accept(fd, NULL, NULL)) == -1) {
            perror("accept failed");
            continue;
        }

        sigprocmask(SIG_BLOCK, &sigiohandleraction.sa_mask, NULL);
        requests[cnt] = create_request(cl);
        if (requests[cnt] != NULL) {
            cnt++;
        }
        sigprocmask(SIG_UNBLOCK, &sigiohandleraction.sa_mask, NULL);
    }
}
