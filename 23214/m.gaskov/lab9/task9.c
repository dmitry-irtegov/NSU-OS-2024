#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Wrong usage");
        return EXIT_FAILURE;
    }

    pid_t pid;

    if ((pid = fork()) == -1) {
        perror("Fork error");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        if (execlp("cat", "cat", argv[1], NULL) == -1)
            perror("Execlp error");
        return EXIT_FAILURE;
    } else {
        if (wait(NULL) != pid) {
            perror("Wait error");
            return EXIT_FAILURE;
        }
        printf("HELLO FROM PARENT\n");
    }
    return EXIT_SUCCESS;
}
