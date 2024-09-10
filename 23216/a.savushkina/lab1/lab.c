#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/time.h>
#include <sys/resource.h>

// struct rlimit {
//     rlim_t rlim_cur;   /* мягкое ограничение */
//     rlim_t rlim_max;   /* жесткое ограничение 
//                          (потолок для rlim_cur) */
// };

int main(int argc, char *argv[])
{
    extern char **environ;
    struct rlimit rlm;

    char options[] = "f:dg:ispuU:cC:vV:"; /* valid options */
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
        case 'f':
            fflg++;
            f_ptr = optarg;
            break;
        case 'g':
            gflg++;
            g_ptr = optarg;
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
            // -i  Печатает реальные и эффективные идентификаторы пользователя и группы.
            // -s  Процесс становится лидером группы. Подсказка: смотри setpgid(2).
            // -p  Печатает идентификаторы процесса, процесса-родителя и группы процессов.
            // -u  Печатает значение ulimit
            // -Unew_ulimit  Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)
            // -c  Печатает размер в байтах core-файла, который может быть создан.
            // -Csize  Изменяет размер core-файла
            // -d  Печатает текущую рабочую директорию
            // -v  Распечатывает переменные среды и их значения
            // -Vname=value  Вносит новую переменную в среду или изменяет значение существующей переменной.

        case '?':
            printf("invalid option is %c\n", optopt);
            invalid++;
        }
    }
    if (fflg)
        printf("f_ptr points to %s\n", f_ptr);
    if (gflg)
        printf("g_ptr points to %s\n", g_ptr);
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
        printf("PID: %d\n", getpid());
        printf("PPID: %d\n", getppid());
        printf("PGID: %d\n", getpgrp());
    if (uflg)
    {
        printf("ulimit value: %ld\n", ulimit(UL_GETFSIZE));
    }
    if (Uflg)
    {
        getrlimit(RLIMIT_CORE, &rlm);
        int ulimit_new = atol(U_ptr);
        if (ulimit_new > rlm.rlim_max){
            printf("Current hard limit is %lu\n", rlm.rlim_max);
            perror("bigger then hard limit.\n");
        }
        ulimit(UL_SETFSIZE, ulimit_new);
    }
    if (cflg)
    {
        getrlimit(RLIMIT_CORE, &rlm);
        printf("Soft max core bytes: %lu\n", rlm.rlim_cur);
        printf("Hard max core bytes: %lu\n", rlm.rlim_max);
    }
    if (Cflg)
    {
        int rlimit_new = atol(C_ptr);
        rlm.rlim_cur = rlimit_new;
        rlm.rlim_max = rlimit_new;
        setrlimit(RLIMIT_CORE, &rlm);
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
    // printf("optind equals %d\n", optind);
    if (optind < argc)
        printf("next parameter = %s\n", argv[optind]);
    return 0;
}