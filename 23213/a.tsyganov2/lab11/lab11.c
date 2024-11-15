#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

int execvpe(const char *file, char *const argv[], char *const envp[]) {
	char** old_env = environ;
	environ = (char **)envp;
	execvp(file, argv);
	environ = old_env;
	return -1;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s cmd\n", argv[0]);
		exit(1);
	}

	char *value = getenv("PATH");
	char *path = (char*) malloc(6*sizeof(char));
	strcpy(path, "PATH=");
	strcat(path, value);
	char *new_env[] = {path, NULL};
	free(path);
	execvpe(argv[1], &argv[1], new_env);
	perror("execvpe");
	exit(-1);
}
