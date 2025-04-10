#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
int main(int argc, char *argv[]){
    
    if(argc < 2){
        perror("Missing arguements");
        exit(1);
    }
    char scheme[9] = {0};
    char valid[9] = "http://";
    for(int i = 0; i < 7; i++){
        if(argv[1][i] == 0 || valid[i] != argv[1][i]){
            perror("Wrong url");
            exit(2);
        }
        scheme[i] = argv[1][i];
    }
    struct hostent *host = gethostbyname(argv[1] + 7);
    if(host == NULL){
        perror("Host not found");
        exit(3);
    }
    int sockd = socket(AF_INET, SOCK_STREAM, 0);
    printf("%s\n", host->h_name);
    printf("%d\n", host->h_length);
    for(char** p = host->h_addr_list; *p!= 0; p++){
        struct in_addr in;
        char **q;
        (void) memcpy(&in.s_addr, *p, sizeof (in.s_addr));
        (void) printf("%s\t%s", inet_ntoa(in), host->h_name);
    }
    exit(0);
}