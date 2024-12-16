#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [program] <args>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int status;
	pid_t child_pid;
	child_pid = fork();

	if (child_pid == -1) {
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}

	if (child_pid == 0) {
		execvp(argv[1], &argv[1]);
		exit(EXIT_FAILURE);
	} else {
		pid_t ret;
		ret = waitpid(child_pid, &status, 0);

		if (ret == -1) {
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}

		if (WIFEXITED(status) != 0) {
			printf("Exit status of child process: %d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("Terminated by signal: %d\n", WTERMSIG(status));
		}
	}
	exit(EXIT_SUCCESS);
}
