#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    int n;
    int fd[2]; 
    pid_t pid;
    char line[MAXLINE];
    FILE* fp;
    
    if (argc != 2) {
        fprintf(stderr, "Correct usage: %s <file>\n", argv[0]);
        return 1;
    }
        
    if (pipe(fd) == -1) {
        perror("pipe failed");
        return 1;
    }

    if ((pid = fork()) == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) { 
        close(fd[1]);
        while ((n = read(fd[0], line, MAXLINE)) > 0) {
            for (int i = 0; i < n; i++) {
                line[i] = toupper(line[i]); 
            }
            write(STDOUT_FILENO, line, n);
        }
        close(fd[0]);
        exit(0); 

    } else {
        close(fd[0]);

        if ((fp = fopen(argv[1], "r")) == NULL){
            perror("fopen failed");
            return 1;
        }

        while (fgets(line, MAXLINE, fp) != NULL) {
            n = strlen(line);
            if (write(fd[1], line, n) != n){
                perror("write failed");
                close(fd[1]); 
                fclose(fp);
                return 1;
            }
                
        }
        close(fd[1]); 
        fclose(fp);
        wait(NULL);
    }

    return 0;
}