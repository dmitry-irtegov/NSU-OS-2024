#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

#define PORT 80
#define SIZE 100
#define BUF_SIZ 8192
#define LINE_BUF 8192

int sockfd = -1;
int parse_url(char *url, char* host, char* path)
{
    char *host_start;
    const char* http = "http:";
    char temp[5] = {0};
    memcpy(temp, url, 5);
    if (strcmp(temp, http)) {
        return 1;
    }

    host_start = strstr(url, "://") + 3;
    
    char *path_start;
    if ((path_start = strchr(host_start, '/')) == NULL)
    {
        strcpy(path, "/");
        strcpy(host, host_start);
    }
    else
    {
        strncpy(host, host_start, path_start - host_start);
        strcpy(path, path_start);
    }
    return 0;
}

void set_non_canonical_mode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);

    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void restore_canonical_mode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void send_http_request(int sockfd, char* path, char* host)
{
    char request[2048];
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    if ((write(sockfd, request, strlen(request))) == -1)
    {
        perror("write error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void signal_handler()
{
    close(sockfd);
    restore_canonical_mode();
    _exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char path[512] = {0};
    char host[512] = {0};
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation error");
        return -1;
    }

    if (parse_url(argv[1], host, path)) {
        close(sockfd);
        fprintf(stderr, "wrong url\n");
        exit(EXIT_FAILURE);
    }
    struct hostent *server = gethostbyname(host);
    if (server == NULL)
    {
        fprintf(stderr, "gethostbyname error; exit code = %d\n", h_errno);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("signal error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);

    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    send_http_request(sockfd, path, host);
    set_non_canonical_mode();

    char buffer[BUF_SIZ + 1] = {0};
    char line_buffer[LINE_BUF + 1] = {0};
    int lines_printed = 0;
    int bytes_read = 0;
    fd_set read_fds;
    int max_fd;
    int printing = 1;
    int full_buffer = 0;
    int bytes_to_print = 0;
    int rest_bytes = BUF_SIZ;
    int write_shift = 0;
    int read_shift = 0;
    char read_buffer[SIZE];
    int header_end = 0;
    if (sockfd > STDIN_FILENO)
    {
        max_fd = sockfd;
    }
    else
    {
        max_fd = STDIN_FILENO;
    }
    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            close(sockfd);
            restore_canonical_mode();
            break;
        }

        if (FD_ISSET(sockfd, &read_fds))
        {
            
            if (rest_bytes >= SIZE) {

                bytes_read = read(sockfd, read_buffer, SIZE);
                if (bytes_read <= 0) {
                    if (bytes_read == -1)
                    {
                        perror("read error");
                        break;
                    }
    
                    if (bytes_read == 0) {
                        if (rest_bytes >= BUF_SIZ) {
                            break;
                        }
                        
                    }
                }

                else {
                    bytes_to_print = bytes_read;
                    
                    rest_bytes -= bytes_read;
                    for (int i = 0; i < bytes_read; i++) {
                        buffer[write_shift] = read_buffer[i];
                        write_shift++;
                        write_shift %= BUF_SIZ;
                    }
                
                     
                }
                
            } 
            char* buffer_ptr;
            char* line_buffer_ptr;
            char* end;
            
            if (!header_end) {
                char* header_end_ptr = strstr(buffer, "\r\n\r\n");
                if (header_end_ptr != 0) {
                    header_end = 1;
                    buffer_ptr = header_end_ptr + 4;
                    line_buffer_ptr = line_buffer;
                    read_shift = buffer_ptr - buffer;
                    end = buffer + strlen(buffer);
                    rest_bytes += read_shift;
                } 
            }
            else {
                buffer_ptr = buffer + read_shift;
                line_buffer_ptr = line_buffer;
                end = buffer + read_shift + bytes_to_print;
            }            
            while (buffer_ptr < end && printing && rest_bytes < BUF_SIZ)
            {   
                if (*buffer_ptr == '\n')
                {
                    *line_buffer_ptr = '\0';
                    read_shift++;
                    read_shift %= BUF_SIZ;
                    printf("%s\n", line_buffer);
                    rest_bytes++;
                    lines_printed++;
                    line_buffer_ptr = line_buffer;
                }
                else
                {
                    *line_buffer_ptr = *buffer_ptr;
                    read_shift++;
                    read_shift %= BUF_SIZ;
                    rest_bytes++;
                    line_buffer_ptr++;
                }

                buffer_ptr++;

                if (buffer_ptr == end && strlen(line_buffer) > 0)
                {
                    *line_buffer_ptr = '\0';
                    printf("%s", line_buffer);
                }
                
                if (lines_printed >= 25)
                {   
                    printing = 0;
                    printf("Press space to scroll down...\n");
                }
            }
            
            
        
            
            
        }
        
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            char ch = 0;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == ' ') {
                    printing = 1;
                    
                    lines_printed = 0;
                }
                
            }
        }
    }
    
    restore_canonical_mode();
    close(sockfd);
    return 0;
}
