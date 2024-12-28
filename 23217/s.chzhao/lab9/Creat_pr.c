#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file_to_cat>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        // 创建子进程失败
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // 子进程代码
        printf("Child process: Executing 'cat' command on file %s...\n", argv[1]);
        execlp("cat", "cat", argv[1], (char *)NULL);

        // 如果 execlp 失败，才会执行以下代码
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        // 父进程代码
        printf("Parent process: Waiting for child process to finish...\n");

        // 父进程打印一些文本
        printf("Parent process: This is a sample message.\n");
        printf("Parent process: Another message.\n");
        printf("Parent process: The final message before child exits.\n");

        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Parent process: Child process exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Parent process: Child process did not exit normally\n");
        }

        // 子进程退出后，打印父进程的最后一行
        printf("Parent process: Final message after child process exited.\n");
    }

    return 0;
}
