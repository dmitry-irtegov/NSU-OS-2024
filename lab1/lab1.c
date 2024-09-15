#define PATH_LEN 200

#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ulimit.h>


int main(int argc, char* argv[]) {
	int opt;
	struct rlimit core_f;
	extern char** environ;
	char** env;
	long int new_ulimit;
	char* cwd;

	if (argc < 2) {
		fprintf(stderr, "No arguments\n");
		exit(0);
			
	}

	while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
		switch (opt) {
		case('i'):
			printf("Real userid = %d\n", getuid());
			printf("Effective userid = %d\n", geteuid());
			printf("Real groupid = %d\n", getgid());
			printf("Effective groupid = %d\n", getegid());
			break;

		case('s'):
			setpgid(0, 0);
			break;

		case('p'):
			printf("Process number = %d\n", getpid());
			printf("Parent process number = %d\n", getppid());
			printf("Group process number = %d\n", getpgid(0));
			break;

		case('u'):
			printf("ulimit = %ld\n", ulimit(UL_GETFSIZE) * 512);
			break;

		case('U'):
			new_ulimit = atol(optarg);
			if (new_ulimit % 512 != 0) {
				fprintf(stderr, "Value for ulimit has to be divisible by 512\n");
				break;
			}
			new_ulimit = ulimit(UL_SETFSIZE, new_ulimit / 512);
			if (new_ulimit == -1) {
				fprintf(stderr, "Сan't change ulimit\n");
			}
			break;

		case('c'):
			getrlimit(RLIMIT_CORE, &core_f);
			printf("Сore size = %ld\n", core_f.rlim_cur);
			break;

		case('C'):
			getrlimit(RLIMIT_CORE, &core_f);
			core_f.rlim_cur = atol(optarg);
			if (setrlimit(RLIMIT_CORE, &core_f) == -1)
				fprintf(stderr, "Can't change core\n");
			break;

		case('d'):
			cwd = getcwd(NULL, 200);
			if (cwd == NULL) {
				fprintf(stderr, "Couldn't get current directory");
				break;
			}
			printf("Сurrent working directory is: %s\n", cwd);
			free(cwd);
			break;

		case('v'):
			env = environ;
			while (*env) {
				printf("%s\n", *env);
				env++;
			}
			break;

		case('V'):
			putenv(optarg);
			break;

		case('?'):
			fprintf(stderr, "Unknown option: -%c\n", optopt);
			break;
		}
	}
	exit(0);
}