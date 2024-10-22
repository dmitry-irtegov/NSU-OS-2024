#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>


int main() {
	int fd[2];
	int bytes_read = 0;
	int wait_status;
	pid_t pid;
	char text[100] = "qweRTYioopaSDFGJKLzxcbnmGKbJggFFf";
	char recieved[100];
	if (pipe(fd) == -1) {
		perror("Pipe create failure");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if (pid == -1) {
		perror("Fork failure");
		exit(EXIT_FAILURE);
	}
	else {
		if (pid == 0) {
			close(fd[1]);
			if ((bytes_read = read(fd[0], recieved, strlen(text) + 1)) == -1) {
				perror("read failure");
				exit(EXIT_FAILURE);
			}
			//printf("bytes_read = %d\n", bytes_read);
			for (int i = 0; i < bytes_read; i++) {
				recieved[i] = toupper(recieved[i]);
			}
			printf("%s\n", recieved);
			close(fd[0]);
			exit(EXIT_SUCCESS);
		}
		else {
			close(fd[0]);
			if (write(fd[1], text, strlen(text) + 1) == -1) {
				perror("write error");
				exit(EXIT_FAILURE);
			}
			if (wait(&wait_status) == -1) {
				perror("wait failure");
				exit(EXIT_FAILURE);
			}
			close(fd[1]);
			exit(EXIT_SUCCESS);
		}

	}
}
