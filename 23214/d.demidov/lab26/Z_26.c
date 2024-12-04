#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define BATCH_SIZE 1024  

int main() {
    FILE *pipe_input;      
    char buffer[BATCH_SIZE];  
    int wait_status;     

    pipe_input = popen("cat lower.txt", "r");
    if (pipe_input == NULL) {
        perror("popen error");
        exit(1);
    }
    while (fgets(buffer, BATCH_SIZE, pipe_input) != NULL) {
        for (int i = 0; i < BATCH_SIZE && buffer[i] != '\0'; i++) {
            buffer[i] = toupper(buffer[i]);  
        }
        printf("%s", buffer);  
    }
    if (pclose(pipe_input) == -1) {
        perror("pclose error");
        exit(1);
    }
    exit(0);
}