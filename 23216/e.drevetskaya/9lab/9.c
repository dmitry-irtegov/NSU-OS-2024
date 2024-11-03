#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    // Проверяем, передан ли аргумент с именем файла
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    pid_t pid;
    if ((pid = fork()) == -1) {//не получилось сделать подпроцесс
        perror("failed fork subprocess");
        return 1;
    }

    switch (pid) {
        case -1:
            perror("fork dead");
            return 1;
            break;

        case 0: // Child process
            printf("Child here\n");
            if(execlp("cat", "cat", argv[1], NULL) == -1){
                perror("cat dead");//если бы cat норм отработал то это бы и не показалось
                exit(1);
                break;
            }
        default: // Parent process
            if (wait(NULL) == -1) {
                perror("Couldn't wait for the pid to complete");
                exit(1);
            }

            printf("\nThis is end\n");
            break;
    }

    return 0;
}
