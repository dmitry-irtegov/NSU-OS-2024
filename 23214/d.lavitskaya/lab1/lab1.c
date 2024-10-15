#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>

extern char **environ;

int main(int argc, char *argv[])
{
    char options[] = "ispuU:cC:dvV:";
    int opt;
    char opts[argc-1];
    char* opt_args[argc-1];
    int opt_index = 0;
    opterr = 0;
    

    while ((opt = getopt(argc, argv, options)) != -1) {
        if(opt == '?'){
            opts[opt_index] = optopt;
        }
        else{
            opts[opt_index] = opt;
        }
        
        opt_args[opt_index] = optarg ? strdup(optarg) : NULL;
        opt_index ++;

    }

    char cwd[PATH_MAX];
    char *arg_ptr, *endptr, *name_val_ptr;
    long ulimit_long;

    struct rlimit limit;


    for (int i = opt_index - 1; i >= 0; i--) {
        switch(opts[i]) 
        {
            case 'i':{
                printf("UID: %d EUID: %d GID: %d EGID: %d\n", getuid(), geteuid(), getgid(), getegid());
                break;
            }
                
            case 's':{
                pid_t pid = getpid();
                if (setpgid(pid, pid)){
                    perror("setpgid failed");
                }
                else{
                    printf("process %d is leader of the process group\n", pid);
                }
                break;
            }

            case 'p':{
                pid_t pid = getpid();
                pid_t ppid = getppid();
                pid_t pgid;
                if((pgid=getpgid(pid))==-1){
                    perror("getpgid failed");
                }
                else{
                    printf("PID: %d PPID: %d PGID: %d\n", pid, ppid, pgid);
                }
                
                break;
            }

            case 'd':{
                if (getcwd(cwd, sizeof(cwd)) == NULL) {
                    perror("getcwd failed");
                }
                else{
                    printf("Current working directory: %s\n", cwd);
                }
                break;
            }

            case 'v':{
                while(*environ){
                    printf("%s\n", *environ);
                    *environ++;
                }
                break;
            }

            case 'u':{
                printf("File size soft limit: %ld 512-byte blocks\n", ulimit(UL_GETFSIZE));
                break;
            }

            case 'c':{
                if (getrlimit(RLIMIT_CORE, &limit) == 0) {
                   printf("Core file size: (soft) %lu bytes, (hard) %lu bytes\n", limit.rlim_cur, limit.rlim_max);
                }
                else{
                    perror("getrlimit failed");
                }
                break;
            }

            case 'U':{
                if(opt_args[i] == NULL) {
                    fprintf(stderr, "There is no argument for -U\n");
                    break;
                }

                arg_ptr = opt_args[i];
                ulimit_long = strtol(arg_ptr, &endptr, 10); 

                if (*endptr != '\0' || ulimit_long < 0) {
                    fprintf(stderr, "Invalid argument for -U: must be a non-negative integer.\n");
                    break;
                }

                if(ulimit(UL_SETFSIZE, ulimit_long)==-1){
                    perror("ulimit failed");
                }
                else{
                    printf("File size soft and hard limits set to %ld 512-byte blocks\n", ulimit_long);
                }
                free(opt_args[i]);
                break;  
            }

             case 'C': {
                 if(opt_args[i] == NULL) {
                    fprintf(stderr, "There is no argument for -C\n");
                    break;
                }

                arg_ptr = opt_args[i];
                ulimit_long = strtol(arg_ptr, &endptr, 10);
                
                if (*endptr != '\0' || ulimit_long < 0 ) {
                    fprintf(stderr, "Invalid argument for -C: must be a non-negative integer.\n");
                    break;
                }
            
                limit.rlim_cur = ulimit_long;

                if (!setrlimit(RLIMIT_CORE, &limit)) {
                    printf("New core file size soft limit set to: %lu bytes\n", limit.rlim_cur);
                }
                else{
                    perror("setrlimit failed");
                }
                free(opt_args[i]);
                break;   
            }

            case 'V':{
                if(opt_args[i] == NULL) {
                    fprintf(stderr, "There is no argument for -V\n");
                    break;
                }

                name_val_ptr = opt_args[i];
                char *equals_sign = strchr(name_val_ptr, '=');

                if (equals_sign && equals_sign != name_val_ptr && *(equals_sign + 1) != '\0'){
                    *equals_sign = '\0';
                    char *name = name_val_ptr;
                    char *value = equals_sign + 1;

                    if (!setenv(name, value, 1)) { 
                        printf("Environment variable set: %s=%s\n", name, value);
                    } 
                    else{
                        perror("setenv failed");
                    }

                }
                else {
                    fprintf(stderr, "Invalid argument for -V: must be in the format name=value.\n");
                }
                free(opt_args[i]);
                break;

            }
            
            default: {
                fprintf(stderr, "invalid option -- '%c'\n", opts[i]);
                break;
            }

        }
    }

    exit(0);
}
