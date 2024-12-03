#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

extern char** environ;

void printUserIDs() {
    printf("The real user ID: %d.\n", getuid());
    printf("The effective user ID: %d.\n", geteuid());
}

void printGroupIDs() {
    printf("The real group ID: %d.\n", getgid());
    printf("The effective group ID: %d.\n", getegid());
}

void printProcID() {
    pid_t procID = getpid();
    printf("The process ID: %d.\n", procID);
}

void printParentProcID() {
    pid_t parentID = getppid();
    printf("The parent process ID: %d.\n", parentID);
}

void printGroupProcID() {
    pid_t groupID = getpgid(0);
    printf("The group process ID: %d.\n", groupID);
}

void printULimit() {
    long limit = ulimit(UL_GETFSIZE);
    if (limit == -1 && errno != 0) {
        perror("Error while getting ulimit");
    } else {
        printf("ulimit is %ld.\n", limit);
    }
}

void setULimit(long newULimit) {
    long error = ulimit(UL_SETFSIZE, newULimit);
    if (error == -1 && errno != 0) {
        perror("Error while setting ulimit");
    } else {
        printf("Success! New ulimit is %ld.\n", newULimit);
    }
}

void printCoreLimit() {
    struct rlimit rlim;
    int error = getrlimit(RLIMIT_CORE, &rlim);
    if (error == -1 && errno != 0) {
        perror("Error while getting RLIMIT_CORE");
    } else {
        printf("Cur core limit is %ld.\n", rlim.rlim_cur);
        printf("Max core limit is %ld.\n", rlim.rlim_max);
    }
}

void setCoreLimit(long newCoreSize) {
    struct rlimit rlim;
    rlim.rlim_cur = newCoreSize;
    rlim.rlim_max = newCoreSize;

    int error = setrlimit(RLIMIT_CORE, &rlim);
    if (error == -1) {
        perror("Error while setting RLIMIT_CORE");
    } else {
        printf("Success! Core limit is %ld.\n", newCoreSize);
    }
}

void printDirectory() {
    char buffer[PATH_MAX+1] = { 0 };
    char* error = getcwd(buffer, PATH_MAX);
    
    if (error == NULL) {
        perror("Error getting current directory");
    } else {
        buffer[PATH_MAX] = '\0';
        printf("Current work directory is '%s'.\n", buffer);
    }
}

void printEnvVars() {
    printf("Environment variables:\n");
    for (int i = 0; environ[i] != NULL; i++) {
        printf("\t%s\n", environ[i]);
    }
}

void newOrChangeEnvVar(char* str) {
    int error = putenv(str);
    if (error == 0) {
        printf("putenv() was successful.\n");
    } else {
        perror("Error when creating or changing environment variable");
    }
}

int main(int argc, char* argv[])
{
    char options[] = "ispuU:cC:dvV:";
    int c = 0;
    int argCnt = 0;

    long newULimit = 0;
    long newCoreLimit = 0;
    char* endptr;

    if (argc <= 1) {
        printf("No arguments.\n"); 
    }
    else {
        while ((c = getopt(argc, argv, options)) != EOF) {
            argCnt++;
            switch (c) {
            case 'i':
                printUserIDs();
                printGroupIDs();
                break;
            case 's':
                setpgid(0, 0);
                break;
            case 'p':
                printProcID();
                printParentProcID();
                printGroupProcID();
                break;
            case 'u':
                printULimit();
                break;
            case 'U':
                newULimit = strtol(optarg, &endptr, 10);
                if (endptr != optarg) {
                    setULimit(newULimit);
                }
                else {
                    printf("Argument '%s' isn't number.\n", optarg);
                }
                break;
            case 'c':
                printCoreLimit();
                break;
            case 'C':
                newCoreLimit = strtol(optarg, &endptr, 10);
                if (endptr != optarg) {
                    setCoreLimit(newCoreLimit);
                }
                else {
                    printf("Argument '%s' isn't number.\n", optarg);
                }
                break;
            case 'd':
                printDirectory();
                break;
            case 'v':
                printEnvVars();
                break;
            case 'V':
                newOrChangeEnvVar(optarg);
                break;
            case '?':
                printf("invalid option is '%c'.\n", optopt);
                break;
            }
        }

        if (argCnt == 0) {
            printf("No options.\n");
        }
    }
    
    return 0;
}
