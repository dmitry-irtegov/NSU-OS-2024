#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //mode_t
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

// Функция для печати прав доступа в человеко-читаемом формате
void print_permissions(mode_t mode)
{
    char type;

    if (S_ISDIR(mode))
        type = 'd'; // Каталог
    else if (S_ISREG(mode))
        type = '-'; // Обычный файл
    else
        type = '?'; // Все остальное

    printf("%c", type);

    // Права владельца
    printf("%c", (mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (mode & S_IXUSR) ? 'x' : '-');

    // Права группы
    printf("%c", (mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (mode & S_IXGRP) ? 'x' : '-');

    // Права остальных
    printf("%c", (mode & S_IROTH) ? 'r' : '-');
    printf("%c", (mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (mode & S_IXOTH) ? 'x' : '-');
}

// Функция для получения имени файла из пути
const char *get_filename(const char *path)
{
    return basename((char *)path); // Возвращает имя файла из пути
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file>...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        struct stat file_stat;

        // Получаем информацию о файле
        if (lstat(argv[i], &file_stat) == -1)
        {
            perror("lstat");
            continue;
        }

        // Печать прав доступа
        print_permissions(file_stat.st_mode);

        // Печать количества ссылок
        printf(" %2ld", (long)file_stat.st_nlink);

        // Получение имени владельца
        struct passwd *pw = getpwuid(file_stat.st_uid);
        if (pw)
        {
            printf(" %-8s", pw->pw_name);
        }
        else
        {
            printf(" %-8d", file_stat.st_uid);
        }

        // Получение имени группы
        struct group *gr = getgrgid(file_stat.st_gid);
        if (gr)
        {
            printf(" %-8s", gr->gr_name);
        }
        else
        {
            printf(" %-8d", file_stat.st_gid);
        }

        // Если файл обычный, печатаем его размер
        if (S_ISREG(file_stat.st_mode))
        {
            printf(" %8ld", (long)file_stat.st_size);
        }
        else
        {
            printf("         "); // Для каталогов оставляем место пустым
        }

        // Печать времени модификации файла
        char time_str[20];
        struct tm *tm_info = localtime(&file_stat.st_mtime);
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
        printf(" %s", time_str);

        // Печать имени файла
        printf(" %s\n", get_filename(argv[i]));
    }

    return 0;
}
