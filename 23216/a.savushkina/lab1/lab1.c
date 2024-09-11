#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(int argc, char *argv[])
{
    extern char **environ;
    struct rlimit rlm;

    char options[] = "dispuU:cC:vV:";
    int c, invalid = 0, dflg = 0, fflg = 0, gflg = 0,
           iflg = 0, sflg = 0, pflg = 0,
           uflg = 0, Uflg = 0, cflg = 0,
           Cflg = 0, vflg = 0, Vflg = 0;
    char *f_ptr, *g_ptr, *U_ptr, *C_ptr, *V_ptr;
    char dir_info[100];
    printf("argc equals %d\n", argc);
    while ((c = getopt(argc, argv, options)) != EOF)
    {
        switch (c)
        {
        case 'd':
            dflg++;
            break;
        case 'i':
            iflg++;
            break;
        case 's':
            sflg++;
            break;
        case 'p':
            pflg++;
            break;
        case 'u':
            uflg++;
            break;
        case 'U':
            Uflg++;
            U_ptr = optarg;
            break;
        case 'c':
            cflg++;
            break;
        case 'C':
            Cflg++;
            C_ptr = optarg;
            break;
        case 'v':
            vflg++;
            break;
        case 'V':
            Vflg++;
            V_ptr = optarg;
            break;
        case '?':
            printf("invalid option is %c\n", optopt);
            invalid++;
        }
    }
    if (iflg)
    {
        printf("Real uid %d\n", getuid());
        printf("Effective uid %d\n", geteuid());
        printf("Real gid %d\n", getgid());
        printf("Effective gid %d\n", getegid());
    }
    if (sflg)
        setpgid(0, 0);

    if (pflg)
    {
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("PGID: %d\n", getpgrp());
    }
    if (Uflg)
    {
        getrlimit(RLIMIT_CORE, &rlm);
        int ulimit_new = atol(U_ptr);
        if (ulimit_new > rlm.rlim_max || ulimit_new < 0)
        {
            printf("Current hard limit is %lu\n", rlm.rlim_max);
            perror("ulimit bigger then hard limit.\n");
            exit(1);
        }
        ulimit(UL_SETFSIZE, ulimit_new, 1);
    }
    if (uflg)
    {
        printf("ulimit value: %ld\n", ulimit(UL_GETFSIZE));
    }
    if (Cflg)
    {
        int rlimit_new = atol(C_ptr);
        rlm.rlim_cur = rlimit_new;
        rlm.rlim_max = rlimit_new;
        setrlimit(RLIMIT_CORE, &rlm);
    }
    if (cflg)
    {
        getrlimit(RLIMIT_CORE, &rlm);
        printf("Soft max core bytes: %lu\n", rlm.rlim_cur);
        printf("Hard max core bytes: %lu\n", rlm.rlim_max);
    }
    if (dflg)
        printf("Directory %s\n", getcwd(dir_info, 100));
    if (vflg)
    {
        for (char **env = environ; *env; env++)
        {
            printf("%s\n", *env);
        }
    }
    if (Vflg)
    {
        putenv(V_ptr);
    }
    if (invalid)
        printf("invalid equals %d\n", invalid);
    if (optind < argc)
        printf("next parameter = %s\n", argv[optind]);
    exit(0);
}