#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>


int main() {
    char* string = "I'm just a simple English string";
    int size = strlen(string) + 1;
    int pipefd[2];

    if(pipe(pipefd) == -1) {
        fprintf(stderr, "Ошибка создания канала\n");
        exit(1);
    }

    pid_t pid = fork();

    if(pid < 0) {
        perror("Ошибка запуска fork\n");
        exit(1);
    } else if (pid > 0) { 
        printf("pipefd[0] descriptor: %d, pipefd[1] descriptor: %d\n", pipefd[0], pipefd[1]);
        close(pipefd[0]);
        printf("Родительский процесс отправляет: %s\n", string);
        if (write(pipefd[1], string, size) == -1) {
            perror("Ошибка записи");
            exit(1);
        }
        close(pipefd[1]);
    } else { 
        close(pipefd[1]);
        char getString[size];
        char *str = getString;
        if (read(pipefd[0], getString, size) == -1) {
            perror("Ошибка чтения");
            exit(1);
        }
        printf("Дочерний процесс отправляет: ");
        ssize_t res;
        while (res = read(pipefd[0], str, size) != 0){
            str = str + res;
        }
        for(int i = 0; i < size - 1; i++) {        
            printf("%c", toupper(getString[i]));
        }
        printf("\n");
        close(pipefd[0]); 
    }

    return 0;
}