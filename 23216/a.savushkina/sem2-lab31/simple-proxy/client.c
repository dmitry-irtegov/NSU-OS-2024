#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PROXY_PORT 8080
#define BUFFER_SIZE 4096

int main()
{
    int sockfd;
    struct sockaddr_in proxy_addr;
    char buffer[BUFFER_SIZE];
    char *requests[] = {
        "GET / HTTP/1.0\r\nHost: example.com\r\nCache-Control: no-cache\n\r\n",
    };
    int num_requests = 100;

    for (int i = 0; i < num_requests; i++) {
        printf("\nREQUEST %d\n", i + 1);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        printf("Socket created\n");

        proxy_addr.sin_family = AF_INET;
        proxy_addr.sin_port = htons(PROXY_PORT);
        if (inet_pton(AF_INET, "127.0.0.1", &proxy_addr.sin_addr) <= 0) {
            perror("inet_pton failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Proxy address set\n");

        if (connect(sockfd, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
            perror("connect failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("Connected to proxy\n");
        send(sockfd, requests[0], strlen(requests[0]), 0);
        printf("Request sent: %s\n", requests[0]);

        ssize_t bytes_received;
        while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
            if (strstr(buffer, "\r\n\r\n")) {
                break;
            }
        }
        printf("Response received\n");
        char *close_command = "CLOSE_CONNECTION";
        send(sockfd, close_command, strlen(close_command), 0);

        close(sockfd);
        printf("Socket closed\n");
    }

    return 0;
}