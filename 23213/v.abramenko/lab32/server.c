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
    struct aiocb req;
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

    request* req = (request*)info->si_value.sival_ptr;
    if (aio_error(&req->req) == 0) {
        req->completed = 1;
        siglongjmp(toprocess, 1);
    }
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
    while (1) {
        sigprocmask(SIG_BLOCK, &sigiohandleraction.sa_mask, NULL);
        if (sigsetjmp(toprocess, 1) != 0) {
            for (int i = 0; i < cnt; i++)
            {
                if (!requests[i]->completed) {
                    continue;
                }
                
                int rc = aio_return(&requests[i]->req);
                if (rc <= 0) {
                    if (rc == -1) {
                        perror("return failed");
                    }
                    char* buf = (char*)requests[i]->req.aio_buf;
                    free(buf);
                    free(requests[i]);
                    requests[i] = requests[cnt - 1];
                    requests[cnt - 1] = NULL; 
                    cnt--;
                    i--;
                } else {
                    char* buf = (char*)requests[i]->req.aio_buf;
                    buf[rc] = 0;
                    for (int j = 0; j < rc; j++) {
                        putchar(toupper((unsigned char)buf[j]));
                    }
                    if (aio_read(&requests[i]->req) == -1) {
                        free(buf);
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
        requests[cnt] = malloc(sizeof(request));
        if (requests[cnt] == NULL) {
            perror("malloc failed");
            close(cl);
            continue;
        }

        requests[cnt]->req.aio_fildes = cl;
        requests[cnt]->req.aio_offset = 0;
        requests[cnt]->req.aio_buf = malloc(BUF_SIZE * sizeof(char));
        if (requests[cnt]->req.aio_buf == NULL) {
            perror("malloc failed");
            free(requests[cnt]);
            requests[cnt] = NULL;
            close(cl);
            continue;
        }
        requests[cnt]->req.aio_nbytes = BUF_SIZE - 1;
        requests[cnt]->req.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        requests[cnt]->req.aio_sigevent.sigev_signo = SIGIO;
        requests[cnt]->req.aio_sigevent.sigev_value.sival_ptr = requests[cnt];
        if (aio_read(&requests[cnt]->req) == -1) {
            perror("aio_read failed");
            char* buf = (char*)requests[cnt]->req.aio_buf;
            free(buf);
            free(requests[cnt]);
            requests[cnt] = NULL;
            close(cl);
            continue;
        }
        cnt++:
        sigprocmask(SIG_UNBLOCK, &sigiohandleraction.sa_mask, NULL);
    }
}
