#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <libgen.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

int main(int argc, char** argv) {
    if (argc == 1) {
        argv[argc++] = ".";
    }
    for(int file = 1; file < argc; file++) {
        struct stat stbuf;
        struct passwd* pw;
        struct group* gr;
        if (lstat(argv[file], &stbuf) == -1) {
            perror("Error accesing file with lstat");
            continue;
        }
        switch(stbuf.st_mode & S_IFMT) {
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
               stbuf.st_mode & S_IRUSR ? 'r' : '-',
               stbuf.st_mode & S_IWUSR ? 'w' : '-',
               stbuf.st_mode & S_IXUSR ? 'x' : '-',
               stbuf.st_mode & S_IRGRP ? 'r' : '-',
               stbuf.st_mode & S_IWGRP ? 'w' : '-',
               stbuf.st_mode & S_IXGRP ? 'x' : '-',
               stbuf.st_mode & S_IROTH ? 'r' : '-',
               stbuf.st_mode & S_IWOTH ? 'w' : '-',
               stbuf.st_mode & S_IXOTH ? 'x' : '-');
        printf("%3u ", (unsigned int)stbuf.st_nlink);
        pw = getpwuid(stbuf.st_uid);
        if (pw == NULL) {
            printf("%-8d ", (int)stbuf.st_uid);
        } else {
            printf("%-8s ", pw->pw_name);
        }
        gr = getgrgid(stbuf.st_gid);
        if (gr == NULL) {
            printf("%-8d ", (int)stbuf.st_gid);
        } else {
            printf("%-8s ", gr->gr_name);
        }
        if ((stbuf.st_mode & S_IFMT) == S_IFREG) {
            printf("%7jd ", (intmax_t)stbuf.st_size);
        } else {
            printf("        ");
        }
        char time_buf[64];
        time_t current_time;
        time(&current_time);
        if (current_time - stbuf.st_mtime < 6 * 30 * 24 * 60 * 60 && stbuf.st_mtime < current_time) {
            strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", localtime(&stbuf.st_mtime));
        } else {
            strftime(time_buf, sizeof(time_buf), "%b %e  %Y", localtime(&stbuf.st_mtime));
        }
        printf("%s ", time_buf);
        printf("%s", basename(argv[file]));
        if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
            char link_target[PATH_MAX];
            ssize_t len = readlink(argv[file], link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(" -> %s", link_target);
            }
        }
        printf("\n");
    }
    exit(0);
}