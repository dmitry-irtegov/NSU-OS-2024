#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
	int wstatus;
	printf("current id = %d; parent id = %d\n", getpid(), getppid());
	pid_t child_id = fork();
	printf("after fork: current id = %d; parent id = %d\n", getpid(), getppid());
	printf("child_id = %d\n", child_id);
	if (child_id == -1) {
		perror("failed to fork");
		exit(EXIT_FAILURE);
	}
	else {
		if (child_id == 0) {
			execlp("cat", "cat", "test.txt", (char *)0);
			perror("execlp error occured");
			exit(EXIT_FAILURE);
		}
		else {
			if (wait(&wstatus) == -1) {
				perror("wait error");
				exit(EXIT_FAILURE);

			}
			else {
				printf("wait success \n");
				exit(EXIT_SUCCESS);
			}

		}
	}
}
