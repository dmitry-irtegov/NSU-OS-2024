#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
	pid_t child_pid = fork();

	if (child_pid == -1) {
		perror("fork() failed");
		exit(EXIT_FAILURE);
	} else if (child_pid == 0) {
		if (
			system("cat text.txt") == -1
		) {
			perror("system() failed");
			exit(EXIT_FAILURE);
		}
	} else {
		if (
			waitpid(child_pid, NULL, 0) == -1
		) {
			perror("waitpid() failed");
			exit(EXIT_FAILURE);
		}
		printf("Hello from main process!");
	}

	exit(EXIT_SUCCESS);
}
