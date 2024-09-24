#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <limits.h>
#include <ulimit.h>
#include <sys/resource.h>
#include <string.h>

extern char *optarg; 
extern char **environ; 
extern int opterr, optopt; 

int main(int argc, char* argv[]){ 
    if(argc == 1) { 
        printf("Usage: %s [options]\n\
Options:\n\
    -i                  print real and effective user and groups ID\n\
    -s                  process becames leader of process group\n\
    -p                  print IDs of process, its parent and group\n\
    -u                  print ulimit\n\
    -Unew_ulimit        set new ulimit\n\
    -c                  print size of core file\n\
    -C size             set new size of core file\n\
    -d                  print current directory\n\
    -v                  print enviroment variable\n\
    -Vname=value        set new enviroment variable\n", argv[0]);
        exit(EXIT_FAILURE); 
    } 

    opterr = 0; //turn off getopt's errors
    char *opts = "vdsipucU:V:C:"; 
    int opt; 
    int pgid;
    struct rlimit rlp;
    long new_limit;
    while((opt = getopt(argc, argv, opts)) != -1) {
        switch(opt) { 
            case 'v':  
                for (char **p = environ; *p; p++){
                    printf("%s\n", *p);
                }
                break; 
            case 'V': 
                if (putenv(optarg)){
                    perror("failed putenv");
                } 
                else{
                    puts("new enviroment variable was added");
                }
                break; 
            case 'd':
                printf("current directory = %s\n", getcwd(NULL, 100));
                break;
            case 's':
                if (setpgid(0, 0)){
                    perror("failed setpgid");
                }
                break;
            case 'i':
                printf("uid = %i, euid = %i, gid = %i, egid = %i\n", getuid(), geteuid(), getgid(), getegid());
                break;
            case 'p':
                pgid = getpgrp();
                if (pgid == -1){
                    perror("failed getpgrp");
                    continue;
                }
                printf("pid = %d, ppid = %d, pgid = %d\n", getpid(), getppid(), pgid);
                break;
            case 'u':
                if (getrlimit(RLIMIT_FSIZE, &rlp)){
                    perror("failed getrlimit");
                } 
                else{
                    printf("soft = %ld, hard = %ld\n", rlp.rlim_cur, rlp.rlim_max);
                }
                break;
            case 'U':
                new_limit = atol(optarg);
                if (new_limit == 0){
                    perror("failed atol");
                }
                else if (getrlimit(RLIMIT_FSIZE, &rlp)){
                    perror("failed getrlimit");
                } 
                else{
                    rlp.rlim_cur = new_limit;
                    if (setrlimit(RLIMIT_FSIZE, &rlp)){
                        perror("failed to execute setrlimit");
                    }
                }
                break;
            case 'c':
                if (getrlimit(RLIMIT_CORE, &rlp)){
                    perror("failed to execute getrlimit");
                }
                else{
                    printf("soft = %ld, hard = %ld\n", rlp.rlim_cur, rlp.rlim_max);
                }
                break;
            case 'C':
                new_limit = atol(optarg);
                if (new_limit == 0){
                    fprintf(stderr, "invalid argument for -U");
                }
                else if (getrlimit(RLIMIT_CORE, &rlp)){
                    perror("failed to execute getrlimit");
                } 
                else{
                    rlp.rlim_cur = new_limit;
                    if (setrlimit(RLIMIT_CORE, &rlp)){
                        perror("failed to execute setrlimit");
                    }
                }
                break;
            case '?':
                fprintf(stderr, "%c: invalid option or insufficiest number of argument\n", optopt);
                break;
            case ':':
                fprintf(stderr, "%c: missing argument", opt);
                break;
        } 

    } 
    exit(EXIT_SUCCESS); 
}