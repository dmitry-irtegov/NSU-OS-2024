#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>


extern int optind, optopt;
extern char *optarg;
extern char **environ;

int main(int argc, char *argv[]) {
    char options[] = "ispuU:cC:dvV:";
    int opt;
    while ((opt = getopt(argc, argv, options)) != -1) {
        switch (opt) {
            case 'i': {
                printf("Ruid: %d, Euid: %d\n", getuid(), geteuid());
                printf("Rgid: %d, Egid: %d\n", getgid(), getegid());
                break;
            }
            case 's': {
                if (setpgid(0, 0) == -1) {
                    perror("setpgid");
                } else {
                    printf("setpgid was successful\n");
                }
                break;
            }
            case 'p': {
                int pid = getpid();
                int ppid = getppid();
                int pgid = getpgrp();
                printf("pid: %d, ppid: %d, pgid: %d\n", pid, ppid, pgid);
                break;
            }
            case 'u': {
                long ul = ulimit(UL_GETFSIZE);
                if(ul == -1) {
                    perror("ulimit");
                    break;
                }
                printf("ulimit value: %ld\n", ul);
                break;
            }
            case 'U': {
                long ul = ulimit(UL_GETFSIZE);
                if(ul == -1) {
                    perror("ulimit");
                    break;
                }
                printf("previous ulimit value: %ld\n", ul);
                long newul = atol(optarg);
                if(ulimit(UL_SETFSIZE, newul) == -1) {
                    perror("ulimit");
                    break;
                }
                printf("new ulimit value: %ld\n", newul);
                break;
            }
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("getrlimit");
                    break;
                }
                printf("core file limits:\n\tsoft: %ld\n\thard: %ld\n", rl.rlim_cur, rl.rlim_max);
                break;
            }
            case 'C': {
                long newcore = atol(optarg);
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("getrlimit");
                    break;
                }

                rl.rlim_cur = newcore;
                if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("setrlimit");
                    break;
                }

                printf("new core file size limit: %ld\n", newcore);
                break;
            }
            case 'd': {
                char path[PATH_MAX];
                if (getcwd(path, sizeof(path)) != NULL) {
                    printf("current directory: %s\n", path);
                } else {
                    perror("getcwd");
                }
                break;
            }
            case 'v': {
                printf("eviron variables:\n");
                for(int i = 0; environ[i] != NULL; i++) {
                    printf("\t%s\n", environ[i]);
                }
                break;
            }
            case 'V': {
                if (optarg == NULL || strchr(optarg, '=') == NULL) {
                    fprintf(stderr, "need use name=value argument\n");
                    break;
                }

                if (putenv(optarg) == -1) {
                    perror("putenv");
                    break;
                }

                printf("new environment variable: %s\n", optarg);
                break;
            }
            case '?': {
                printf("Invalid option: -%c\n", optopt);
                break;
            }
        }
    }

    return 0;
}
