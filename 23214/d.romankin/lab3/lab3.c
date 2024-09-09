#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {
	printf("Real ID = %d\n", getuid());
	printf("Effective ID = %d\n", geteuid());
	FILE* file = fopen("test.txt", "r+");
	if (file == NULL){
		perror("ERROR");
	}
	else {
		fclose(file);
	}
	setuid(getuid());
	printf("Real ID = %d\n", getuid());
	printf("Effective ID = %d\n", geteuid());
	file = fopen("test.txt", "r+");
	if (file == NULL){
		perror("ERROR");
	}
	else {
		fclose(file);
	}
	return 0;
}
