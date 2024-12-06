#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

char get_file_type(mode_t mode) {
    if (S_ISDIR(mode)) return 'd';
    if (S_ISREG(mode)) return '-';
    return '?';
}

void get_permissions(mode_t mode, char *permissions) {
    permissions[0] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[3] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[6] = (mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (mode & S_IXOTH) ? 'x' : '-';
    permissions[9] = '\0';
}

const char *get_filename(const char *path) {
    const char *name = strrchr(path, '/');
    return name ? name + 1 : path;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        struct stat file_stat;
        if (lstat(argv[i], &file_stat) == -1) {
            perror(argv[i]);
            continue;
        }

        char permissions[10];
        get_permissions(file_stat.st_mode, permissions);

        char file_type = get_file_type(file_stat.st_mode);

        struct passwd *owner = getpwuid(file_stat.st_uid);
        struct group *group = getgrgid(file_stat.st_gid);

        char *owner_name = owner ? owner->pw_name : "unknown";
        char *group_name = group ? group->gr_name : "unknown";

        char mod_time[20];
        strftime(mod_time, sizeof(mod_time), "%d-%m-%Y %H:%M", localtime(&file_stat.st_mtime));

        printf("%c%s %3u %-8s %-8s %8ld %s %s\n",
               file_type,
               permissions,
               file_stat.st_nlink,
               owner_name,
               group_name,
               S_ISREG(file_stat.st_mode) ? file_stat.st_size : 0,
               mod_time,
               get_filename(argv[i]));
    }

    return EXIT_SUCCESS;
}
