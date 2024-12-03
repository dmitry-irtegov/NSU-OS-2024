#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    
    char *filename = "/home/Hortes/longfile.txt";
    
    // Создаём подпроцесс
    pid_t pid = fork();
    //pid>0 будет ждать завершения процесса с указанным pid
    //pid==0 , будет ждать завершения любого дочернего принадлежащего той же группе процессов, что и вызывающий процесс.
    //pid<-1 , будет ждать любого дочернего, чья группа процессов равна абсолютному значению pid
    //pid==-1, будет ждать завершения любого дочернего
    
    if (pid < 0) {
        // Ошибка при создании процесса
        perror("Ошибка при создании дочернего процесса");
        return 1;
    } else if (pid == 0) {
        // Это дочерний процесс
        printf("Дочерний процесс: вывод содержимого файла с помощью cat\n");

        // Выполняем команду cat для отображения файла
        execlp("cat", "cat", filename, (char *)NULL);

        // Если execlp завершился с ошибкой
        perror("Ошибка при выполнении cat");
        return 1;
    } else {
        // Это родительский процесс
        printf("Родительский процесс: сообщение до завершения дочернего процесса\n");

        // Ожидаем завершения дочернего процесса
        waitpid(pid, NULL, 0);

        // После завершения дочернего процесса
        printf("Родительский процесс: сообщение после завершения дочернего процесса\n");
    }

    return 0;
}