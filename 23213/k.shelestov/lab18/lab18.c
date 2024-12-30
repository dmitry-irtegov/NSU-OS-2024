#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
#include <locale.h>
#include <langinfo.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for(int files = 1; files < argc; files++) {
        struct stat sbuf;
        struct passwd* pwuid;
        struct group* gruid;

        if (lstat(argv[files], &sbuf) == -1) {
            perror("Error accessing file");
            continue;
        }

        switch(sbuf.st_mode & S_IFMT) {
            case S_IFDIR:
                printf("d");
                break;
            case S_IFREG:
                printf("-");
                break;
            case S_IFIFO:
            	printf("p");
            	break;
            case S_IFCHR:
            	printf("c");
            	break;
            case S_IFLNK:
            	printf("l");
            	break;
            case S_IFSOCK:
            	printf("s");
            	break;
            case S_IFBLK:
            	printf("b");
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
        
        printf("%3u ", sbuf.st_nlink);

        pwuid = getpwuid(sbuf.st_uid);
        if (pwuid == NULL) {
            printf("%-8d ", sbuf.st_uid);
        } else {
            printf("%-8s ", pwuid->pw_name);
        }

        gruid = getgrgid(sbuf.st_gid);
        if (gruid == NULL) {
            printf("%-8d ", sbuf.st_gid); 
        } else {
            printf("%-8s ", gruid->gr_name);
        }

        if ((sbuf.st_mode & S_IFMT) == S_IFREG) {
            printf("%11jd ", (intmax_t)sbuf.st_size);
        } else {
            printf("%11s ", "");
        }

        
        printf("%.24s %s\n", ctime(&sbuf.st_mtime), basename(argv[files])); 
    }
    return EXIT_SUCCESS; 
}

