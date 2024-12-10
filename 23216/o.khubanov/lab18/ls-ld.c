#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Получить тип файла
char get_file_type(mode_t mode) {
    if (S_ISDIR(mode)) return 'd';
    if (S_ISREG(mode)) return '-';
    return '?';
}

// Получить строку с правами доступа
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

// Получить имя владельца
const char* get_owner(uid_t uid) {
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : "UNKNOWN";
}

// Получить имя группы
const char* get_group(gid_t gid) {
    struct group *gr = getgrgid(gid);
    return gr ? gr->gr_name : "UNKNOWN";
}

// Получить строку времени модификации
void get_mod_time(time_t mtime, char *buffer, size_t size) {
    struct tm *timeinfo = localtime(&mtime);
    strftime(buffer, size, "%Y-%m-%d %H:%M", timeinfo);
}

// Получить имя файла без пути
const char* get_file_name(const char *path) {
    const char *file_name = strrchr(path, '/');
    return file_name ? file_name + 1 : path;
}

// Печать информации о файле
void print_file_info(const char *path, size_t owner_width, size_t group_width) {
    struct stat file_stat;
    if (lstat(path, &file_stat) == -1) {
        perror("lstat");
        return;
    }

    char type = get_file_type(file_stat.st_mode);

    char permissions[10];
    get_permissions(file_stat.st_mode, permissions);

    nlink_t n_links = file_stat.st_nlink;

    const char *owner = get_owner(file_stat.st_uid);
    const char *group = get_group(file_stat.st_gid);

    char size[20] = "";
    if (S_ISREG(file_stat.st_mode)) {
        snprintf(size, sizeof(size), "%ld", file_stat.st_size);
    }

    char mod_time[20];
    get_mod_time(file_stat.st_mtime, mod_time, sizeof(mod_time));

    const char *file_name = get_file_name(path);

    printf("%c%s %3u %-*s %-*s %8s %s %s\n",
       type, permissions, n_links, 
       (int)owner_width, owner,
       (int)group_width, group,
       size, mod_time, file_name);

           
}

// Рассчитать максимальные ширины колонок
void calculate_column_widths(int argc, char *argv[], size_t *owner_width, size_t *group_width) {
    *owner_width = 0;
    *group_width = 0;

    for (int i = 1; i < argc; i++) {
        struct stat file_stat;
        if (lstat(argv[i], &file_stat) == -1) {
            continue; // Игнорировать ошибки
        }

        const char *owner = get_owner(file_stat.st_uid);
        const char *group = get_group(file_stat.st_gid);

        size_t owner_len = strlen(owner);
        size_t group_len = strlen(group);

        if (owner_len > *owner_width) {
            *owner_width = owner_len;
        }
        if (group_len > *group_width) {
            *group_width = group_len;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    size_t owner_width, group_width;
    calculate_column_widths(argc, argv, &owner_width, &group_width);

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i], owner_width, group_width);
    }

    return 0;
}

