#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    pid = fork();

    if (pid < 0)
    {
        perror("Error");
        exit(1);
    } else if (pid == 0){
        //все че надо делаем
        execlp("cat","cat", "file.txt", NULL); //если всё так то просто подменяет наш дочерний процесс новым процессом и поэтому дочерний завершится.
        exit(1); //если че то пошло не так и дочерний не подменился

    } else {
        wait(NULL);
        printf("This is parent process");
    }

    exit(0);
}