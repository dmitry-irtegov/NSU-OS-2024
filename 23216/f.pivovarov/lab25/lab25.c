#include <sys/types.h>
#include <sys/wait.h>
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
            close(pipeFDs[0]);
            char *text = "aBoba123";
            if (write(pipeFDs[1], text, strlen(text)) != (int) strlen(text)) {
                perror("Cannot write text correctly");
                close(pipeFDs[1]);
                exit(EXIT_FAILURE);
            }
            close(pipeFDs[1]);
            printf("Child is end\n");
            exit(EXIT_SUCCESS);
        default: ;
            // Parent process code
            char buffer[256];
            int readedBytes;
            wait((int *)&process_id);
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
