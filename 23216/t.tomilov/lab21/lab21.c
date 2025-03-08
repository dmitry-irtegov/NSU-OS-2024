#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

volatile sig_atomic_t counter = 0;

void boop(int sig){
	if (sig == SIGQUIT){
		char quit[] = "\nQUIT SIGNAL COUGHT!\n";
		write(1, quit, strlen(quit));
		char num[20];
		sprintf(num, "%d", counter);
		write(1, num, strlen(num));
		_exit(EXIT_SUCCESS);
	}
	counter++;
	write(1,"\a", 1);
}

int main(){
	sigset(SIGINT, boop);
	sigset(SIGQUIT, boop);
	while(1){
		pause();
	}
}