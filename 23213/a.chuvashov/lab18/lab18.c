#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>

int main(int argc, char** argv) {
    
    for(int files = 1; files < argc; files++) {

        struct stat sbuf;
        struct passwd* pwuid;
        struct group* gruid;

        if (stat(argv[files], &sbuf) == -1) {
            perror(argv[files]);
            exit(1);
        }

        switch(sbuf.st_mode & S_IFMT) {
            case S_IFDIR:
                printf("d");
                break;
            case S_IFREG:
                printf("-");
                break;
            default:
                printf("?");
                break;
            }

        printf("%c%c%c%c%c%c%c%c%c ", 
                sbuf.st_mode & S_IRUSR ? 'r' : '-',
                sbuf.st_mode & S_IWUSR ? 'w' : '-',
                sbuf.st_mode & S_IXUSR ? 'x' : '-',
                sbuf.st_mode & S_IRGRP ? 'r' : '-',
                sbuf.st_mode & S_IWGRP ? 'w' : '-',
                sbuf.st_mode & S_IXGRP ? 'x' : '-',
                sbuf.st_mode & S_IROTH ? 'r' : '-',
                sbuf.st_mode & S_IWOTH ? 'w' : '-',
                sbuf.st_mode & S_IXOTH ? 'x' : '-');

        pwuid = getpwuid(sbuf.st_uid);
        gruid = getgrgid(sbuf.st_uid);
        
        printf("%lu %s %s ", sbuf.st_nlink, pwuid->pw_name, gruid->gr_name);
        if ((sbuf.st_mode & S_IFMT) == S_IFREG) {
            printf("%ld ", sbuf.st_size);
        }
        printf("%.24s %s\n", ctime(&sbuf.st_mtime), basename(argv[files]));
    }
    exit(0);
}
