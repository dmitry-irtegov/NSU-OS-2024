#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <poll.h>
#include <signal.h>

struct sockaddr_un sock_struct;
int server_sock;

void clear(){
    close(server_sock);
    unlink(sock_struct.sun_path);
}

void add_descriptor(int sock, struct pollfd* fds, nfds_t* ndfs){
    fds[*ndfs].fd = sock;
    fds[*ndfs].events = POLLIN;
    *ndfs += 1;
}

void remove_descriptor(int index, struct pollfd* fds, nfds_t* ndfs){
    fds[index] = fds[*ndfs-1];
    *ndfs -= 1;
}

void work_text(int index, char* text, int *ready_sockets_size, struct pollfd* fds, nfds_t* ndfs ) {
    int size;
    size = read(fds[index].fd, text, 5);
    if(size == -1){
        perror("read error");
        close(fds[index].fd);
        exit(3);
    }
    if(size == 0){
        close(fds[index].fd);
        *ready_sockets_size -= 1;
        remove_descriptor(index, fds, ndfs);
    }
    for (int i = 0; i < size; i++){
        text[i] = toupper(text[i]);
    }

    printf("%.*s", size, text);
    fflush(stdout);
}

void make_connection(struct pollfd* fds, nfds_t* ndfs){
    int accepted_sock;
    if ((accepted_sock = accept(server_sock, 0, 0)) == -1){
        perror("accept error");
        clear();
        exit(4);
    }
    add_descriptor(accepted_sock, fds, ndfs);
}

void sigcatch(){
    clear();
   _exit(0);
}

int main(){
    char text[BUFSIZ];
    static char* path = "choose_own_path";
    int ready_sockets_size;
    struct pollfd fds[BUFSIZ];
    nfds_t nfds = 1;
    signal(SIGINT, sigcatch);

    memset(&sock_struct, 0, sizeof (sock_struct));
    sock_struct.sun_family = AF_UNIX;
    strncpy(sock_struct.sun_path, path, sizeof(sock_struct.sun_path) - 1);

    if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0) ) == -1){
        perror("socket error");
        exit(1);
    }
    if ((bind(server_sock, (struct sockaddr *) &sock_struct, sizeof(sock_struct))) == -1){
        perror("bind error");
        close(server_sock);
        exit(2);
    }
    if ((listen(server_sock, SOMAXCONN)) == -1){
        perror("listen error");
        clear();
        exit(3);
    }

    fds[0].fd = server_sock;
    fds[0].events = POLLIN;

    while (1){
        if ((ready_sockets_size = poll(fds, nfds, -1)) == -1 ){
            perror("poll error");
            clear();
            exit(6);
        }

        if (fds[0].revents & POLLIN && nfds < BUFSIZ){
            ready_sockets_size--;
            make_connection(fds, &nfds);
        }
        for(int i = 1; i < (int) nfds && ready_sockets_size > 0; i++){
            if(fds[i].revents & POLLIN)
                work_text(i, text, &ready_sockets_size, fds, &nfds);
        }
    }
}

