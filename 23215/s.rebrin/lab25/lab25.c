#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#define MSGSIZE 20

int main(int argc, char** argv) {

	if (argc < 2) {
		exit(0);
	}

	int fd[2];
	pid_t pid;
	char* msgout = argv[1];
	char* msgin;
	int len = strlen(argv[1]) + 1;

	msgin = (char*)malloc(len);

	if (pipe(fd) == -1) {
		perror(argv[0]);
		exit(1);
	}

	if ((pid = fork()) > 0) {  /* parent */
		write(fd[1], msgout, len);
	}
	else if (pid == 0) {      /* child */
		read(fd[0], msgin, len);
		for (int i = 0; msgin[i] != '\0'; i++) {
			msgin[i] = toupper(msgin[i]);
		}
		puts(msgin);
	}
	else {          /* cannot fork */
		perror(argv[0]);
		exit(2);
	}

	exit(0);
}
