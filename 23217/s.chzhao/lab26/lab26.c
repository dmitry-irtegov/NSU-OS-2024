#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int main() {
    const char *message = "Hello, Pipe! Mixed CASE text here.\n";
    FILE *pipe_stream;

    pipe_stream = popen("ls -a", "w");
    if (pipe_stream == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    fprintf(pipe_stream, "%s", message);
    // 关闭子进程的输入输出流
    if (pclose(pipe_stream) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }

    return 0;
}
