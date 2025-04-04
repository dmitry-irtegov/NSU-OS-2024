#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }
    int status;
    pid_t pid;

    if ((pid = fork()) == -1) {
        perror("Error: fork failed");
        return 1;
    }
    if (pid == 0) {
        // 使用 execvp 执行命令，将 argv[1] 后的参数传给该命令
        if (execvp(argv[1], &argv[1]) == -1) {
            perror("Error: failed to execute command");
            exit(127);
        }
    }

    if (waitpid(pid, &status, 0) == -1) {
        perror("Error: waitpid failed");
        return 1;
    }

    // 检查子进程是否正常退出
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Child process exited with code: %d\n", exit_code);
    } 
    // 检查子进程是否被信号终止
    else if (WIFSIGNALED(status)) {
        int term_signal = WTERMSIG(status);
        printf("Child process terminated by signal: %d\n", term_signal);
    } 
    return 0;
}
