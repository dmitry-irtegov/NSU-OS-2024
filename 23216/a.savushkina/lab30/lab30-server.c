#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#define BUF_SIZE 500

int main(int argc, char *argv[]) {
    int sfd, s;
    char buf[BUF_SIZE];
    ssize_t nread;
    socklen_t peer_addrlen;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    struct sockaddr_storage peer_addr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    memset(&hints, 0, sizeof(hints));
    memset(&buf, 0, sizeof(buf));

    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; /* Success */

        close(sfd);
    }

    freeaddrinfo(result); /* No longer needed */

    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        char host[NI_MAXHOST], service[NI_MAXSERV];

        peer_addrlen = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                         (struct sockaddr *)&peer_addr, &peer_addrlen);
        if (strcmp(buf, "end") == 0) {
            printf("end of server\n");
            exit(EXIT_SUCCESS);
        }
        if (nread == -1)
            continue; /* Ignore failed request */

        s = getnameinfo((struct sockaddr *)&peer_addr,
                        peer_addrlen, host, NI_MAXHOST,
                        service, NI_MAXSERV, NI_NUMERICSERV);

        for (size_t i = 0; i < strlen(buf); i++) {
            buf[i] = toupper(buf[i]);
        }
        if (strlen(buf) < 500){
            buf[strlen(buf)] = '\n';
        }
        if (s == 0) {
            if (write(fileno(stdin), &buf, strlen(buf)) == -1) {
                perror("error in write");
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        }
    }
}
