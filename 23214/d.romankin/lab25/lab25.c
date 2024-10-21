#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>


int main() {
	int fd[2];
	
	pid_t pid;

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
			if (close(fd[1]) == -1) {
				printf("error\n");
			}
			char recieved[BUFSIZ];
			ssize_t bytes_read;
			while((bytes_read = read(fd[0], recieved, BUFSIZ)) > 0) {
				for (int i = 0; recieved[i] != 0; i++) {
					recieved[i] = toupper(recieved[i]);
				}
				printf("%s", recieved);
			}
			if (bytes_read == -1) {
				perror("read failure");
				exit(EXIT_FAILURE);
			}
			printf("\n");
			close(fd[0]);
			exit(EXIT_SUCCESS);
		}
		else {
			close(fd[0]);
			int wait_status;
			char text[BUFSIZ] = "qweRTYioopaSDFGJKLzxcbnmGKbJggFFf";
			if (write(fd[1], text, strlen(text) + 1) == -1) {
				perror("write error");
				exit(EXIT_FAILURE);
			}
			close(fd[1]);
			if (wait(&wait_status) == -1) {
				perror("wait failure");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}

	}
}

