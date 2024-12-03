#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == -1) {

        perror("fork");
        exit(EXIT_FAILURE);
    }
 
    if (pid == 0) {

        execlp("cat", "cat", "longfile.txt", (char *)NULL);

        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
  
        printf("Это сообщение от родителя\n");
   
        waitpid(pid, NULL, 0);
       
        printf("Подпроцесс завершен, родитель теперь завершает работу\n");
    }

    return 0;
}
