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

char* socket_path = "./socket";

void close_handler(int sig) {
	unlink(socket_path);
	_exit(0);
}

sigjmp_buf toprocess;
struct aiocb* completed;

void sigiohandler(int signo, siginfo_t* info, void* context){
    if (signo != SIGIO || info->si_signo != SIGIO){
        return;
    }

    struct aiocb* req = (struct aiocb*)info->si_value.sival_ptr;
    if (aio_error(req) == 0) {
        completed = req;
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
    while (1) {
        if (sigsetjmp(toprocess, 1) == 1){
            int rc = aio_return(completed);
            if (rc <= 0) {
                if (rc == -1) {
                    perror("return failed");
                }
                char* buffer = (char*)completed->aio_buf;
                free(buffer);
                close(completed->aio_fildes);
                free(completed);
            } else {
                char* buf = (char*)completed->aio_buf;
                buf[rc] = 0;
                for (int j = 0; j < rc; j++) {
                    putchar(toupper((unsigned char)buf[j]));
                }

                if (aio_read(completed) == -1) {
                    char* buffer = (char*)completed->aio_buf;
                    free(buffer);
                    close(completed->aio_fildes);
                    free(completed);
                }
            }
        }


        if ((cl = accept(fd, NULL, NULL)) == -1){
            perror("accept failed");
            continue;
        }

        
        struct aiocb* req = malloc(sizeof(struct aiocb));
        if (req == NULL) {
            perror("malloc failed");
            close(cl);
            continue;
        }
        req->aio_fildes = cl;
        req->aio_offset = 0;
        req->aio_buf = malloc(BUF_SIZE * sizeof(char));
        if (req->aio_buf == NULL) {
            perror("malloc failed");
            close(cl);
            continue;
        }
        req->aio_nbytes = BUF_SIZE - 1;
        req->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        req->aio_sigevent.sigev_signo = SIGIO;
        req->aio_sigevent.sigev_value.sival_ptr = req;
        if (aio_read(req) == -1) {
            perror("aio_read failed");
            close(cl);
            continue;
        }
    }
}
