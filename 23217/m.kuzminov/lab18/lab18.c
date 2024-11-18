#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>


int main(int argc, char* argv[]) {
    if(argc < 2) {
        fprintf(stderr, "No filename\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++) {
        struct stat status;
        
        if(stat(argv[i], &status) == -1) {
            perror(argv[i]);
            exit(1);
        }

        int type = status.st_mode & S_IFMT;
        if(S_ISDIR(type)) {
            printf("d");
        } else if(S_ISREG(type)) {
            printf("-");
        } else {
            printf("?");
        }


        int for_mode = status.st_mode;
        for(int j = 0; j < 3; j++) {
            int shift = 3 * j;
            if(for_mode & (S_IRUSR>>shift)) {
                printf("r");
            } else {
                printf("-");
            }
            if(for_mode & (S_IWUSR>>shift)) {
                printf("w");
            } else {
                printf("-");
            }
            if(for_mode & (S_IXUSR>>shift)) {
                printf("x");
            } else {
                printf("-");
            }
        }
        
        printf(" %lu",status.st_nlink);

        struct passwd* for_u_name;
        struct group* for_g_name;

        if((for_u_name = getpwuid(status.st_uid)) == NULL) {
            printf("%d ", status.st_uid);
        } else {
            printf(" %s", for_u_name->pw_name);\
        }
        if((for_g_name = getgrgid(status.st_gid)) == NULL) {
            printf("%d ", status.st_gid);
        } else {
            printf(" %s ", for_g_name->gr_name);
        }
        
        if(S_ISREG(type)) {
            printf("%ld ", status.st_size);
        }
        
        struct tm* info = localtime(&status.st_mtime);
        char mon[4];
        strftime(mon, sizeof(mon), "%b", info);
        printf(" %s %d %d:%d ", mon, info->tm_mday, info->tm_hour, info->tm_min); 

        //print basename
        printf("%s\n", basename(argv[i]));
    }

}
