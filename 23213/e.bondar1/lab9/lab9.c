#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [file]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int status;
	pid_t child_pid;
	child_pid = fork();

	if(child_pid == -1) {
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
	if(child_pid == 0) {
		printf("Child process: started with pid: %d\n", child_pid);
		execlp("cat", "cat", argv[1], NULL);

		perror("Failed to execlp() cat");
		exit(EXIT_FAILURE);
	} else {
		printf("Parent process: started child process with pid: %d\n",child_pid);
		if (waitpid(-1, &status, 0) == -1) {
			perror("Failed to waitpid() child");
			exit(EXIT_FAILURE);
		}
		printf("Parent process: child finished execution\n");
	}

	return 0;
}
