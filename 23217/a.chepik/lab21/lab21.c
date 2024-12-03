#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#define BEL "\07"

int ans = 0;

void handler_sigint(int signum) {
	write(STDOUT_FILENO, BEL, 1);
	ans++;

	if (signal(SIGINT, handler_sigint) == SIG_ERR) {
		printf("signal() function returned SIG_ERR.\n");
		exit(-1);
	}
}

void handler_sigquit(int signum) {
	char buffer[13];
	int length = snprintf(buffer, sizeof(buffer), "\n%d\n", ans);

	write(STDOUT_FILENO, buffer, length);
	exit(0);
}

int main() {
	if (signal(SIGINT, handler_sigint) == SIG_ERR) {
		printf("signal() function returned SIG_ERR.\n");
		exit(-1);
	}

	if (signal(SIGQUIT, handler_sigquit) == SIG_ERR) {
		printf("signal() function returned SIG_ERR.\n");
		exit(-1);
	}

	while (1) {
            pause();
        }
}
