#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

int main(int argc, char** argv) {

	if (argc < 2) {
		exit(0);
	}

	int fd[2];
	pid_t pid;
	char* msgout = argv[1];
	char* msgin;
	int len = strlen(argv[1]) + 1;

	msgin = (char*)malloc(21);

	if (pipe(fd) == -1) {
		perror(argv[0]);
		exit(1);
	}

	if ((pid = fork()) > 0) {  /* parent */
		close(fd[0]);
		write(fd[1], msgout, len);
	}
	else if (pid == 0) {      /* child */
		close(fd[1]);
		int rd;
		while ((rd = read(fd[0], msgin, 20)) > 0) {
			for (int i = 0; i < rd; i++) {
				msgin[i] = toupper(msgin[i]);
			}
			write(1, msgin, rd);
		}
		printf("\n");
	}
	else {          /* cannot fork */
		perror(argv[0]);
		exit(2);
	}

	exit(0);
}
