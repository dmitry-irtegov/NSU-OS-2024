#include <sys/types.h>
#include <unistd.h>
#include <ulimit.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[]) {
	int opt;
	struct rlimit rm;
	char* endptr;
	long val;

	while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
		switch (opt) {
		case 'i':
			printf("Real id: user - %d group - %d,\nEffective id: user - %d group - %d\n", getuid(), getgid(), geteuid(), getegid());
			break;

		case 's':
			setpgid(0, 0);
			printf("Your process is leader now!!!!\n");
			break;

		case 'p':
			printf("Process id: %d, Parent process id: %d, Group process id: %d\n", getpid(), getppid(), getpgid(getpid()));
			break;

		case 'u':
			("Ulimit: %ld\n", ulimit(U_GETFSIZE));
			break;

		case 'U':
			endptr = NULL;
			val = strtol(optarg, &endptr, 10);
			long newULimit;

			if (*endptr != '\0' || (errno == ERANGE && (val == LONG_MIN || val == LONG_MAX))) {
				perror("Error: Invalid value for -U: %s\n", optarg);
				exit(1);
			} else if (val < 0 || val > LONG_MAX || ulimit(UL_SETFSIZE, newULimit) == -1) {
				perror("Error: Invalid value for -U: %s\n", optarg);
				exit(1);
			} 
			else {
				printf("Ulimit %ld now!!!\n", new newULimit);
			}

			break;

		case 'c':
			getrlimit(RLIMIT_CORE, &rm);
			printf("Soft limit: %ld, Hard limit: %ld\n", rm.rlim_cur, rm.rlim_max);
			break;

		case 'C':
			getrlimit(RLIMIT_CORE, &rm);
			endptr = NULL;
			val = strtol(optarg, &endptr, 10);

			if (*endptr != '\0' || (errno == ERANGE && (val == LONG_MIN || val == LONG_MAX))) {
				fprintf(stderr, "Error: Invalid value for -U: %s\n", optarg);
				exit(1);
			}

			if (val < 0 || val > LONG_MAX) {
				fprintf(stderr, "Error: Invalid value for -U: %s\n", optarg);
				exit(1);
			}

			if (val > rm.rlim_max) {
				rm.rlim_max = val;
				setrlimit(RLIMIT_CORE, &rm);
			}

			rm.rlim_cur = val;
			setrlimit(RLIMIT_CORE, &rm);
			printf("Soft limit for RLIMIT_CORE is %ld now!!!\n", val);
			break;

		case 'd':
			char cwd[PATH_MAX];
			getcwd(cwd, sizeof(cwd));
			printf("Current Directory: %s\n", cwd);
			break;
		case 'v':
			extern char** environ;
			char** envp = environ;
			while (*envp != NULL) {
				printf("%s\n", *envp);
				envp++;
			}
			break;
		case 'V':
			if (putenv(optarg) == -1) {
				perror("setenv");
				exit(1);
			}
			break;
		}
	}

	return 0;
}
