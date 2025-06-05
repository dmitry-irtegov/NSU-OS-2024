#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#define EXAMPLE "http://example.com"
#define SCREEN_HEIGHT 25 //в количестве строк
#define BUF 666
typedef struct ar{
    char * buf;
    int symsPrinted;
    int limitSize;
    int curSize;
}ar;

int main(int argc, char *argv[]){
    
    if(argc < 2){
        perror("Missing arguement");
        exit(1);
    }
    if(strlen(argv[1]) > 500){
        perror("Too long url");
        exit(2);
    }
    //argv[1] = EXAMPLE;
    
    //парсинг url
    char hostname[BUF] = {0};
    char path[BUF] = {0};
    char portStr[BUF] = {0};
    int port = 80;
    if(sscanf(argv[1], "http://%s", hostname) != 1){
        perror("Wrong url");
        exit(3);
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
        exit(4);
    }
    if(*(host->h_addr_list) == 0){
        perror("No addresses found");
        exit(5);
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
        exit(6);
    }
    if(connect(sock, (struct sockaddr *) &ad, sizeof(struct sockaddr_in)) == -1){
        perror("Failed connecting socket");
        close(sock);
        exit(7);
    }
    printf("connected!\n");

    
    //делаем запрос к серверу
    char request[BUF*2 + 50] = {0};
    sprintf(request, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", path, hostname);
    int reqLen = strlen(request);
    if(write(sock, request, reqLen) != reqLen){
        perror("Failed writing request");
        close(sock);
        exit(8);
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
    ar response_buffer;
    response_buffer.buf = (char*)calloc(2, sizeof(char));
    response_buffer.symsPrinted = 0;
    response_buffer.curSize = 0;
    response_buffer.limitSize = 2;
    
    int linesPrinted = 0;
    int connection_active = 1;
    int last_waiting = 0;
    

    while(response_buffer.buf[response_buffer.symsPrinted] != 0 || connection_active){
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(0, &readFds);
        if(connection_active){
            FD_SET(sock, &readFds);
        }

        if(select(sock+1, &readFds, NULL, NULL, NULL) == -1){
            close(sock);
            tcsetattr(0, TCSANOW, &start_tty);
            free(response_buffer.buf);
            perror("Select failed");
            exit(9);
        }

        if(connection_active && FD_ISSET(sock, &readFds)){
            if(response_buffer.curSize == response_buffer.limitSize - 1){
                response_buffer.limitSize *= 2;
                response_buffer.buf = (char*) realloc(response_buffer.buf, sizeof(char) * response_buffer.limitSize);
            }
            int ret = read(sock, response_buffer.buf + response_buffer.curSize, response_buffer.limitSize - response_buffer.curSize - 1);
            response_buffer.curSize += ret;
            
            if(ret == -1){
                close(sock);
                tcsetattr(0, TCSANOW, &start_tty);
                free(response_buffer.buf);
                perror("Socket read error");
                exit(10);
            }
            else if(ret == 0){
                connection_active = 0;
            }
            else{
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
            while(linesPrinted < SCREEN_HEIGHT && response_buffer.buf[response_buffer.symsPrinted] != 0){
                if(last_waiting){
                    printf("\r                                               \r");
                    last_waiting = 0;
                }
                int i = 0;
                char flag = 0;
                while(response_buffer.buf[response_buffer.symsPrinted + i] != 0 
                    && response_buffer.buf[response_buffer.symsPrinted + i] != '\n'){
                    i++;
                }
                if(response_buffer.buf[response_buffer.symsPrinted + i] == '\n'){
                    flag = 1;
                }
                response_buffer.buf[response_buffer.symsPrinted + i] = 0;
                printf("%s", response_buffer.buf + response_buffer.symsPrinted);
                response_buffer.symsPrinted += i;
                if(flag){
                    printf("\r\n");
                    response_buffer.symsPrinted++;
		    linesPrinted++;
                }
            }
            if(response_buffer.buf[response_buffer.symsPrinted] != 0 && !last_waiting){
                fprintf(stderr, "Press any key to scroll down");
                last_waiting = 1;
            }
        }
    }


    close(sock);
    tcsetattr(0, TCSANOW, &start_tty);
    free(response_buffer.buf);
    exit(0);
}  
