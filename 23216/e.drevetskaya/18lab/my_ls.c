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

int get_information_about_file(const char *filepath, struct stat *file_stat);
void print_information_about_file(struct stat file_stat, const char *filename);
void print_permissions_and_type(mode_t mode);
void print_permissions(mode_t mode);
void print_type(mode_t mode);
void print_user(uid_t uid);
void print_group(gid_t gid);
void print_size(struct stat file_stat);
void print_last_mod_time(struct stat file_stat);
const char *get_filename(const char *path);

int main(int argc, char *argv[]){
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file>...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        struct stat file_stat;
        
        //получаем инф-ю по файлу
        if (get_information_about_file(argv[i], &file_stat) == -1) continue; //переходим к след файлу
        //печатаем инф-ю по файлу
        print_information_about_file(file_stat, argv[i]);
    }

    return 0;
}

// Функция для печати инфы по файлам
void print_information_about_file(struct stat file_stat, const char *filename){
    print_permissions_and_type(file_stat.st_mode); // Печать прав доступа и тип файла
    printf(" %2ld", (long)file_stat.st_nlink);     // Печать количества ссылок
    print_user(file_stat.st_uid);
    print_group(file_stat.st_gid);
    print_size(file_stat);                  // Если файл обычный, печатаем его размер
    print_last_mod_time(file_stat);         // Печать времени модификации файла
    printf(" %s\n", get_filename(filename)); // Печать имени файла
}

// Функция для печати прав доступа и типа файла в человеко-читаемом формате
void print_permissions_and_type(mode_t mode){
    print_type(mode);
    print_permissions(mode);
    
}

void print_type(mode_t mode){
    char type;

    if (S_ISDIR(mode))
        type = 'd'; // Каталог
    else if (S_ISREG(mode))
        type = '-'; // Обычный файл
    else
        type = '?'; // Все остальное

    printf("%c", type);
}

void print_permissions(mode_t mode){
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
const char *get_filename(const char *path){
    return basename((char*)path); // Возвращает имя файла из пути
}

int get_information_about_file(const char *filepath, struct stat *file_stat){

    // Получение информации о файле
    if (lstat(filepath, file_stat) == -1) {
        perror("Error getting information about file");
        return -1; 
    }
    return 0; // Успешное выполнение
}

// Функция для вывода имени (id) пользователя
void print_user(uid_t uid){
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        printf(" %-8s", pw->pw_name); // Имя пользователя
    } else {
        printf(" %-8d", uid); // Выводим UID, если не нашли имя
    }
}

// Функция для вывода имени группы
void print_group(gid_t gid){
    struct group *gr = getgrgid(gid);
    if (gr) {
        printf(" %-8s", gr->gr_name); // Имя группы
    } else {
        printf(" %-8d", gid); // Выводим GID, если не нашли имя
    }
}

// Функция для вывода размера файла
void print_size(struct stat file_stat){
    if (S_ISREG(file_stat.st_mode)) {
        printf(" %8ld", (long)file_stat.st_size); // Печатаем размер для обычного файла
    } else {
        printf("         "); // Для каталогов оставляем место пустым
    }
}

// Функция для вывода времени последнего изменения файла
void print_last_mod_time(struct stat file_stat){
    char time_str[20];
    struct tm *tm_info = localtime(&file_stat.st_mtime);
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
    printf(" %s", time_str);
}