#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;  

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if ((pid = fork()) == 0) {
        // Папина дочка
        execlp("cat", "cat", "longfile.txt", NULL);  
        perror("execlp failed"); 
        exit(EXIT_FAILURE);
    }
    else {
        // Папа
        printf("Parent looking for his child\n");

        wait(NULL);

        printf("Child proc is done\n");
    }

    return 0;
}
