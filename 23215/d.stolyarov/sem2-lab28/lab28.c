#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>

#define SCREEN_HEIGHT 25 //в количестве строк

int main(int argc, char *argv[]){
    
    if(argc < 2){
        perror("Missing arguements");
        exit(1);
    }
    
    //парсинг url
    char hostname[999] = {0};
    char path[777] = {0};
    char portStr[666] = {0};
    int port = 80;
    if(sscanf(argv[1], "http://%s", hostname) != 1){
        perror("Wrong url");
        exit(2);
    }
    for(int i = 0; hostname[i] != 0; i++){
        if(hostname[i] == ':'){
            hostname[i] = 0;
            i++;
            for(int j = 0; hostname[i] != 0 && hostname[i] != '/'; j++){
                portStr[j] = hostname[i];
                i++;
            }
            port = atoi(portStr);
        }
        if(hostname[i] == '/'){
            strcpy(path, hostname + i);
            hostname[i] = 0;
            break;
        }
    }
    printf("hostnane: %s\n", hostname);
    printf("path: %s\n", path);
    printf("port: %d\n", port);

    //находим адрес по имени
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

    //подключаемся к серверу
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
    printf("connected!\n");

    
    //делаем запрос к серверу
    char request[9999] = {0};
    sprintf(request, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", path, hostname);
    if(write(sock, request, 1000) == -1){
        perror("Failed writing request");
        close(sock);
        exit(7);
    }
    printf("request sent!\n");


    //меняем настройки терминала, чтобы выводить поэкранно
    struct termios start_tty;
    tcgetattr(0, &start_tty);

    struct termios new_tty = start_tty;
    new_tty.c_lflag &= ~(ICANON | ECHO);
    new_tty.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_tty);

    //получаем и выводим ответ
    printf("---response---\r\n");
    char response_buf[999999] = {0};
    char *response = response_buf;
    int linesPrinted = 0;
    int connection_active = 1;
    int last_waiting = 0;
    

    while(response[0] != 0 || connection_active){
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(0, &readFds);
        if(connection_active){
            FD_SET(sock, &readFds);
        }

        if(select(sock+1, &readFds, NULL, NULL, NULL) == -1){
            close(sock);
            tcsetattr(0, TCSANOW, &start_tty);
            perror("Select failed");
            exit(8);
        }

        if(connection_active && FD_ISSET(sock, &readFds)){
            char buf[999999] = {0};
            int ret = read(sock, buf, 999999);
            if(ret == -1){
                close(sock);
                tcsetattr(0, TCSANOW, &start_tty);
                perror("Socket read error");
                exit(9);
            }
            else if(ret == 0){
                connection_active = 0;
            }
            else{
                strcat(response, buf);

                if(linesPrinted >= SCREEN_HEIGHT && !last_waiting){
                    fprintf(stderr, "Press any key to scroll down");
                    last_waiting = 1;
                }
            }
        }

        if(FD_ISSET(0, &readFds)){
            char t;
            if(read(0, &t, 1) > 0){
                linesPrinted = 0;
            }
        }

        if(linesPrinted < SCREEN_HEIGHT){
            for(; linesPrinted < SCREEN_HEIGHT && response[0] != 0; linesPrinted++){
                if(last_waiting){
                    printf("\r                                               \r");
                    last_waiting = 0;
                }
                char str[99999] = {0};
                int i = 0;
                for(; response[i] != 0 && response[i] != '\n'; i++){
                    str[i] = response[i];
                }
                response = response + i + 1;
                printf("%s\r\n", str);
            }
            if(response[0] != 0 && !last_waiting){
                fprintf(stderr, "Press any key to scroll down");
                last_waiting = 1;
            }
        }
    }


    close(sock);
    tcsetattr(0, TCSANOW, &start_tty);
    exit(0);
}