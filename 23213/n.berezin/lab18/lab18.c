#include <grp.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>

static char* default_file[] = {".", NULL}; 

int main(int argc, char** argv) {
    char** curr_file;
    if (argc < 2) {
        curr_file = default_file;
    } else {
        curr_file = argv + 1;
    }

    for(; *curr_file != NULL; curr_file++) {
        struct stat file_stat;

        if (lstat(*curr_file, &file_stat) == -1) {
            perror(*curr_file);
            continue;
        }

        char ftype;
        switch(file_stat.st_mode & S_IFMT) {
            case S_IFDIR:
                ftype = 'd';
                break;
            case S_IFREG:
                ftype = '-';
                break;
            default:
                ftype = '?';
                break;
        }

        printf("%c%c%c%c%c%c%c%c%c%c %3u ",
            ftype,
            file_stat.st_mode & S_IRUSR ? 'r' : '-',
            file_stat.st_mode & S_IWUSR ? 'w' : '-',
            file_stat.st_mode & S_IXUSR ? 'x' : '-',
            file_stat.st_mode & S_IRGRP ? 'r' : '-',
            file_stat.st_mode & S_IWGRP ? 'w' : '-',
            file_stat.st_mode & S_IXGRP ? 'x' : '-',
            file_stat.st_mode & S_IROTH ? 'r' : '-',
            file_stat.st_mode & S_IWOTH ? 'w' : '-',
            file_stat.st_mode & S_IXOTH ? 'x' : '-',
            file_stat.st_nlink
        );
        
        struct passwd* pw = getpwuid(file_stat.st_uid);
        if (pw) {
            printf("%-8s ", pw->pw_name);
        } else {
            printf("%-8d ", file_stat.st_uid);
        }

        struct group* gr = getgrgid(file_stat.st_uid);
        if (gr) {
            printf("%-8s ", gr->gr_name);
        } else {
            printf("%-8d ", file_stat.st_gid);
        }

        if ((file_stat.st_mode & S_IFMT) == S_IFREG) {
            printf("%7jd ", (intmax_t)file_stat.st_size);
        } else {
            printf("        ");
        }
        
        printf("%.24s %s\n", ctime(&file_stat.st_mtime), basename(*curr_file));
    }
    
    return 0;
}
