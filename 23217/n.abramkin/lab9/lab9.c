#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    char *filename = "largefile.txt"; 

    pid = fork(); 

    if (pid == -1) {
        perror("Ошибка создания подпроцесса");
        return 1;
    } else if (pid == 0) {
        execlp("cat", "cat", filename, (char *) NULL);
        perror("Ошибка выполнения команды cat");
        exit(1);
    } else {
        wait(NULL);
        printf("Подпроцесс завершен, сообщение от родителя.\n");
    }

    return 0;
}
