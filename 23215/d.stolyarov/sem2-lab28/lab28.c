#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
int main(int argc, char *argv[]){
    
    if(argc < 2){
        perror("Missing arguements");
        exit(1);
    }
    char hostname[999] = {0};
    char path[777] = {0};
    int port = 80;
    if(sscanf(argv[1], "http://%s", hostname) != 1){
        perror("Wrong url");
        exit(2);
    }
    for(int i = 0; hostname[i] != 0; i++){
        if(hostname[i] == '/'){
            strcpy(path, hostname + i);
            hostname[i] = 0;
            break;
        }

    }
    printf("hostnane: %s\n", hostname);
    printf("path: %s\n", path);
    struct hostent *host = gethostbyname(hostname);
    if(host == NULL){
        perror("Host not found");
        exit(3);
    }
    if(*(host->h_addr_list) == 0){
        perror("No addresses found");
        exit(4);
    }
    struct in_addr in;
    memcpy(&in.s_addr, *host->h_addr_list, sizeof (in.s_addr));

    printf("address: %s\n", inet_ntoa(in));

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr = in;
    ad.sin_port = htons(port);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        perror("Failed creating socket");
        exit(5);
    }
    if(connect(sock, (struct sockaddr *) &ad, sizeof(struct sockaddr_in)) == -1){
        perror("Failed connecting socket");
        close(sock);
        exit(6);
    }

    char request[1000] = {0};
    sprintf(request, "GET /%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);
    if(write(sock, request, 1000) == -1){
        perror("Failed writing request");
        close(sock);
        exit(7);
    }

    printf("---response---\n");
    char response[9999] = {0};
    while (read(sock, response, 9999))
    {
        printf("%s", response);
    }
    
    close(sock);
    exit(0);
}