#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h> 

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "error: no text file\n");
		exit(EXIT_FAILURE);
	}

	pid_t pid, w;
	int status;

	if ((pid = fork()) > 0) {
		printf("Parent: waiting for child = %d\n", pid);
		w = wait(&status);
		if(w == -1) {
			perror("failed to wait for child\n");
			exit(EXIT_FAILURE);
		}
		printf("Parent: return value=%d\n", w);
		if(WIFEXITED(status)) {
			printf("child's exit status is: %d\n", WEXITSTATUS(status));
		} else if(WIFSIGNALED(status)) {
			printf("signal is: %d\n", WTERMSIG(status));
		}

	} else if(pid == 0) {
		printf("Child: started with pid: %d\n", pid);
		execlp("cat", "cat", argv[1], (char *)0);

		perror("cat failed");
		exit(EXIT_FAILURE);
	} else {
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
