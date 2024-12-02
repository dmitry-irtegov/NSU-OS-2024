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

#define MY_MAXCONN 5

char* socket_path = "./socket";
int servfd, clfd;
int ch = 0;

void close_and_unlink() {
    close(servfd);
    unlink(socket_path);
}

sigjmp_buf toread;

struct aiocb *current_request;

void create_request(int socket){
    struct aiocb *request = calloc(1, sizeof(struct aiocb));

    request->aio_fildes = socket;
    request->aio_offset = 0;
    request->aio_buf = malloc(BUFSIZ);
    request->aio_nbytes = BUFSIZ;

    request->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    request->aio_sigevent.sigev_signo = SIGIO;

    request->aio_sigevent.sigev_value.sival_ptr = request;

    aio_read(request);
}

static void SIGIO_handler(int signo, siginfo_t *siginfo, void *context){
    if (signo != SIGIO || siginfo->si_signo != SIGIO){
        return;
    }

    struct aiocb *request = siginfo->si_value.sival_ptr;
    if (aio_error(request) == 0){
        current_request = request;
        siglongjmp(toread, 1);
    }
}


void process(struct aiocb *request){
    ssize_t size = aio_return(request);
    if (size == 0){
        ch++;
        write(1, &ch, 1);
        write(1, "EOF, closed connection\n", 23);

        char* buffer = (char*) request->aio_buf;
        free(buffer);

        free(request);

        close(request->aio_fildes);
    }
    
    char* buffer = (char*) request->aio_buf;
    for (ssize_t i = 0; i < size; i++){
        buffer[i] = toupper(buffer[i]);
    }
    write(1, buffer, size);
    aio_read(request);
    
}

int main(){
    if ((servfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
        perror("socket error");
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    if (bind(servfd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(servfd);
        exit(1);
    }

    if (listen(servfd, MY_MAXCONN) == -1){
        perror("listen error");
        exit(1);
    }

    struct sigaction sigiohandleraction;

    sigemptyset(&sigiohandleraction.sa_mask);
    sigaddset(&sigiohandleraction.sa_mask, SIGIO);

    sigiohandleraction.sa_sigaction = SIGIO_handler;
    sigiohandleraction.sa_flags = SA_SIGINFO;
    sigaction(SIGIO, &sigiohandleraction, NULL);


    while (1){
        sigprocmask(SIG_BLOCK, &sigiohandleraction.sa_mask, NULL);
        if (sigsetjmp(toread, 1) == 1){
            process(current_request);
        }
        sigprocmask(SIG_UNBLOCK, &sigiohandleraction.sa_mask, NULL);

        if ((clfd = accept(servfd, NULL, NULL)) == -1){
            perror("accept error");
            continue;
        }
        sigprocmask(SIG_BLOCK, &sigiohandleraction.sa_mask, NULL);
        create_request(clfd);
        sigprocmask(SIG_UNBLOCK, &sigiohandleraction.sa_mask, NULL);
    }
}