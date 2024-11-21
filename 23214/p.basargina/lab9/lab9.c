#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main() {

    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        if (execlp("cat", "cat", "stih.txt", NULL) == -1) {
            perror("Execlp error");
            exit(1);
        }
    } else {
        if (wait(NULL) != pid) {
            perror("Wait failed");
            exit(1);
        }
        printf("THIS IS PARENT :)\n");
    }
    exit(0);
}
