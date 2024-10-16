#include <sys/types.h>
#include <ulimit.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <limits.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    int opt;
    struct rlimit rm;
    char* endptr;
    long val;
    char** envp;
    extern char** environ;
    char cwd[PATH_MAX];

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
            printf("Ulimit: %ld\n", ulimit(UL_GETFSIZE));
            break;

        case 'U':
            errno = 0;
            endptr = NULL;
            val = strtol(optarg, &endptr, 10);

            if (*endptr != '\0' || (errno == ERANGE && (val == LONG_MIN || val == LONG_MAX))) {
                fprintf(stderr, "Error: Invalid value for -U: %s\n", optarg);
                exit(1);
            }
            else if (val < 0 || ulimit(UL_SETFSIZE, val) == -1) {
                perror("Error setting ulimit");
                exit(1);
            }
            else {
                printf("Ulimit %ld now!!!\n", val);
            }
            break;

        case 'c':
            getrlimit(RLIMIT_CORE, &rm);
            printf("Soft limit: %ld, Hard limit: %ld\n", rm.rlim_cur, rm.rlim_max);
            break;

        case 'C':
            getrlimit(RLIMIT_CORE, &rm);
            errno = 0;
            endptr = NULL;
            val = strtol(optarg, &endptr, 10);

            if (*endptr != '\0' || (errno == ERANGE && (val == LONG_MIN || val == LONG_MAX))) {
                fprintf(stderr, "Error: Invalid value for -C: %s\n", optarg);
                exit(1);
            }

            if (val < 0) {
                fprintf(stderr, "Error: Invalid value for -C: %s\n", optarg);
                exit(1);
            }

            if (val > rm.rlim_max) {
                rm.rlim_max = val;
                if (setrlimit(RLIMIT_CORE, &rm) == -1) {
                    perror("Error setting hard limit");
                    exit(1);
                }
            }

            rm.rlim_cur = val;
            if (setrlimit(RLIMIT_CORE, &rm) == -1) {
                perror("Error setting soft limit");
                exit(1);
            }
            printf("Soft limit for RLIMIT_CORE is %ld now!!!\n", val);
            break;

        case 'd':
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Current Directory: %s\n", cwd);
            }
            else {
                perror("getcwd() error");
            }
            break;

        case 'v':
            for (envp = environ; *envp != NULL; envp++) {
                printf("%s\n", *envp);
            }
            break;

        case 'V':
            if (putenv(optarg) == -1) {
                perror("putenv failed");
                exit(1);
            }
            break;
        }
    }

    return 0;
}
