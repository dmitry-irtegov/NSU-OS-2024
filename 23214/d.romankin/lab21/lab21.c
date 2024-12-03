#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int count = 0;
char buffer[100];
void sigIntHandler() {
	count++;
	write(1, "\a", 1);
	if (signal(SIGINT, sigIntHandler) == SIG_ERR) {
		perror("SIGINT handler error");
		_exit(EXIT_FAILURE);
	}
}

void sigQuitHandler() {
	sprintf(buffer, "\ntotal signals = %d\n", count);
	write(1, buffer, strlen(buffer) + 1);
	_exit(EXIT_SUCCESS);
	
}

int main() {
	if (signal(SIGINT, sigIntHandler) == SIG_ERR) {
		perror("SIGINT handler error");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGQUIT, sigQuitHandler) == SIG_ERR) {
		perror("SIGINT handler error");
		exit(EXIT_FAILURE);
	}
	while(1) {
		pause();
	}
}
