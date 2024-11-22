#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

void closePipe(const int pipeFDs[2]);

int main() {
    // Opens pipe
    printf("Opens pipe\n");
    int pipeFDs[2];
    if (pipe(pipeFDs) == -1) {
        perror("Troubles with pipe");
        exit(EXIT_FAILURE);
    }
    
    // Forks the current process
    printf("Try to fork()\n");
    pid_t process_id = fork();

    switch (process_id) {
        case -1:
            perror("Can't fork()");
            closePipe(pipeFDs);
            exit(EXIT_FAILURE);
        case 0:
            // Forked process code
            printf("Inside forked\n");
            char write_buffer[256];
            char *text = "aBoba123";
            for (int i = 0; i < (int) strlen(text); i++) {
                if (((i % 256 == 0) && (i != 0)) || i == (int) strlen(text) - 1) {
                    switch (write(pipeFDs[1], write_buffer, 256)) {
                        case -1:
                            perror("Cannot write text correctly");
                            closePipe(pipeFDs);
                            exit(EXIT_FAILURE);
                        case 256:
                            break;
                        default:
                            printf("Child is end\n");
                            exit(EXIT_SUCCESS);
                    }
                }
                write_buffer[i % 256] = text[i];
            }
            printf("Child is end\n");
            //close(pipeFDs[1]);
            exit(EXIT_SUCCESS);
        default: ;
            // Parent process code
            char buffer[256];
            int readedBytes;
            close(pipeFDs[1]);
            while ((readedBytes = read(pipeFDs[0], buffer, 256)) != 0) {
                if (readedBytes == -1) {
                    perror("Troubles in read");
                    close(pipeFDs[0]);
                    exit(EXIT_FAILURE);
                }
                
                for (int i = 0; i < readedBytes; i++) {
                    buffer[i] = (char)toupper(buffer[i]);
                }
                printf("from child: %s\n", buffer);
            }

            printf("parent is end\n");
            close(pipeFDs[0]);
            exit(EXIT_SUCCESS);
        }
}

void closePipe(const int pipeFDs[2]) {
    close(pipeFDs[0]);
    close(pipeFDs[1]);
}
