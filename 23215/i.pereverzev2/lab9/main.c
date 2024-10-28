#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    pid_t chpid = fork();
    if (chpid == -1){
        perror("unable to create child process");
        return 1;
    }
    if (chpid == 0) {
        execlp("cat", "cat", "testfile", NULL);
        perror("cat wasn't executed");
        return 2;
    } else {
	if (waitpid(chpid, NULL, 0) == -1) {
            perror("waitpid error");
            return 3;
        } else {
            printf("sample text from parent process\n");
        }
    }
    return 0;
}
