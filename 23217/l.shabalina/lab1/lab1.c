#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <errno.h>

#define PATH_SIZE 256

extern char **environ;

int main(int argc, char **argv, char **envp) {
    const char options[] = "ispuU:cC:dvV:";

    if (argc == 1) {
        printf("options not found\n");
        exit(EXIT_SUCCESS);
    }

    int curr = 0;
    long new_ulimit;
    long new_climit;
    struct rlimit rlim;
    char buf[PATH_SIZE];

    for (int i = argc - 1; i > 0; i--) {
        optind = i; 
        while ((curr = getopt(argc, argv, options)) != -1) {
            switch (curr) {
                case 'i':
                    printf("real UID:\t%d\n", getuid());
                    printf("effective UID:\t%d\n", geteuid());
                    printf("real GID:\t%d\n", getgid());
                    printf("effective GID:\t%d\n\n", getegid());
                    break;
                case 's':
                    if (setpgid(0, 0) == -1) {
                        perror("setpgid fail");
                    }
                    break;
                case 'p':
                    printf("PID:\t%d\n", getpid());
                    printf("PPID:\t%d\n", getppid());
                    printf("PGID:\t%d\n\n", getpgid(0));
                    break;
                case 'u':
                    printf("ulimit:\t%ld\n\n", ulimit(UL_GETFSIZE));
                    break;
                case 'U':
                    printf("%s\n", optarg);
                    new_ulimit = atol(optarg);
                    ulimit(UL_SETFSIZE, new_ulimit);
                    break;
                case 'c':
                    if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
                        perror("getrlimit fail");
                    } else {
                        printf("сur RLIMIT_CORE:\t%ld\n", rlim.rlim_cur);
                        printf("max RLIMIT_CORE:\t%ld\n\n", rlim.rlim_max);
                    }
                    break;
                case 'C':
                    for (size_t j = 0; j < strlen(optarg); j++) {
                        if (!isdigit(optarg[j])) {
                            fprintf(stderr, "invalid input for -C option: %s\n", optarg);
                            exit(EXIT_FAILURE);
                        }
                    }
                    new_climit = atol(optarg); 
                    rlim.rlim_cur = new_climit;
                    if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
                        perror("setrlimit fail");
                    }
                    break;
                case 'd':
                    if (getcwd(buf, sizeof(buf)) == NULL) {
                        perror("getcwd fail");
                    } else {
                        printf("%s\n\n", buf);
                    }
                    break;
                case 'v':
                    for (char** i = environ; *i != NULL; ++i) {
                        printf("%s\n", *i);
                    }
                    printf("\n");
                    break;
                case 'V':
                    if (putenv(optarg) != 0) {
                        perror("putenv fail");
                    }
                    break;
                default:
                    printf("Command not found\n\n");
                    break;
            }
        }
    }

    exit(EXIT_SUCCESS);
}

