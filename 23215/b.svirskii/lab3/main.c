#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	printf("user id: %d\n", getuid()); 
	printf("effective used id: %d\n", geteuid());
	
	FILE *file = fopen("smth.txt", "r");
	if (file == NULL) {
		perror("Can't open file smth.txt");
	} else {
		fclose(file);
	}

	if (seteuid(getuid()) == -1) {
		perror("seteuid() returned with error\n");
	}

	printf("user id: %d\n", getuid());
	printf("effective used id: %d\n", geteuid());
	
	file = fopen("smth.txt", "r");
	if (file == NULL) {
		perror("Can't open file smth.txt");
	} else {
		fclose(file);
	}

	return 0;
}
