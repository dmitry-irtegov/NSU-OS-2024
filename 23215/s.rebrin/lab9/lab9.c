#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    pid_t pid;  

    if (argc == 0) {
        perror("need file name");
        exit(1);
    }
    if ((pid = fork()) == 0) {
        // Папина дочка
        execlp("cat", "cat", argv[1], NULL);
        perror("execlp failed"); 
        exit(1);
    }
    else {
        // Папа
        printf("Parent looking for his child\n");

        wait(NULL);

        printf("Child proc is done\n");
    }

    return 0;
}
