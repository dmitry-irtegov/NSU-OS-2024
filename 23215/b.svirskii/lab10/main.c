#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Invalid arguments count!\n");
	}

	pid_t child_pid = fork();

	if (child_pid == -1) {
		perror("fork() failed\n");
		exit(EXIT_FAILURE);
	} else if (child_pid == 0) {
		if (
			execvp(argv[1], argv + 1)
		) {
			perror("execvp ends with errors\n");
			exit(EXIT_FAILURE);
		}
	} else {
		int stat_lock;
		if (
			waitpid(child_pid, &stat_lock, 0) == -1
		) {
			perror("waitpid() failed\n");
			exit(EXIT_FAILURE);
		} else {
			printf("child process exit status: %d\n", WEXITSTATUS(stat_lock));
		}
		printf("Hello from main process!\n");
	}

	exit(EXIT_SUCCESS);
}
