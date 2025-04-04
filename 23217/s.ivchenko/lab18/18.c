#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void print_file_info(const char *filepath) {
    struct stat file_stat;
    char type;
    char permissions[10] = "---------";
    struct passwd *pw;
    struct group *gr;
    char time_buf[20];
    char size_buf[20] = "";

    if (stat(filepath, &file_stat) == -1) {
        perror(filepath);
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        type = 'd';
        snprintf(size_buf, sizeof(size_buf), "%ld", file_stat.st_size);
    } else if (S_ISREG(file_stat.st_mode)) {
        type = '-';
        snprintf(size_buf, sizeof(size_buf), "%ld", file_stat.st_size);
    } else {
        type = '?';
    }

    // определение прав доступа
    if (file_stat.st_mode & S_IRUSR) permissions[0] = 'r';
    if (file_stat.st_mode & S_IWUSR) permissions[1] = 'w';
    if (file_stat.st_mode & S_IXUSR) permissions[2] = 'x';
    if (file_stat.st_mode & S_IRGRP) permissions[3] = 'r';
    if (file_stat.st_mode & S_IWGRP) permissions[4] = 'w';
    if (file_stat.st_mode & S_IXGRP) permissions[5] = 'x';
    if (file_stat.st_mode & S_IROTH) permissions[6] = 'r';
    if (file_stat.st_mode & S_IWOTH) permissions[7] = 'w';
    if (file_stat.st_mode & S_IXOTH) permissions[8] = 'x';

    pw = getpwuid(file_stat.st_uid); // имя владельца
    gr = getgrgid(file_stat.st_gid); // имя группы

    // время последней модификации
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", localtime(&file_stat.st_mtime));

    printf("%c%s %lu %s %s %s %s %s\n",
           type, permissions, file_stat.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?",
           size_buf,
           time_buf, filepath);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return EXIT_SUCCESS;
}

// напишите программу - аналог команды ls -ld. Для каждого своего аргумента эта команда должна распечатывать:
// Биты состояния файла в воспринимаемой человеком форме:
// d если файл является каталогом
// - если файл является обычным файлом
// ? во всех остальных случаях
// Три группы символов, соответствующие правам доступа для хозяина, группы и всех остальных:
// r если файл доступен для чтения, иначе -
// w если файл доступен для записи, иначе -
// x если файл доступен для исполнения, иначе -
// Количество связей файла
// Имена собственника и группы файла (совет - используйте getpwuid и getgrgid).
// Если файл является обычным файлом, его размер. Иначе оставьте это поле пустым.
// Дату модификации файла (используйте mtime).
// Имя файла (если было задано имя с путем, нужно распечатать только имя).
 
// Желательно, чтобы поля имели постоянную ширину, т.е. чтобы листинг имел вид таблицы.
// Совет - используйте printf.
