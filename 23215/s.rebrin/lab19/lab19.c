#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

int match(const char* pattern, const char* str) {
    while (*pattern && *str) {
        if (*pattern == '*') {

            while (*pattern == '*') pattern++;
            if (!*pattern) return 1;

            while (*str) {
                if (match(pattern, str)) return 1;
                str++;
            }
            return 0;
        }
        else if (*pattern == '?' || *pattern == *str) {
            pattern++;
            str++;
        }
        else {
            return 0;
        }
    }

    while (*pattern == '*') pattern++;
    return !*pattern && !*str;
}

int main() {
    char pattern[NAME_MAX + 2];
    int rd;
    if ((rd = read(0, pattern, NAME_MAX + 1)) > NAME_MAX) {
        printf("Too large\n");
        while (getchar() != '\n');
        return 0;
    }
    printf("%d %d", rd, NAME_MAX);

    if (rd > 0 && pattern[rd - 1] == '\n') {
        pattern[rd - 1] = '\0';  // Заменяем '\n' на '\0'
    }
    else {
        pattern[rd] = '\0';  // Строка завершается корректно
    }

    DIR* dir = opendir(".");
    struct dirent* entry;
    int found = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (match(pattern, entry->d_name)) {
            printf("%s\n", entry->d_name);
            found = 1;
        }
    }

    closedir(dir);

    if (!found) {
        printf("%s\n", pattern);
    }

    return 0;
}


