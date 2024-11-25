#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define MAX_PATTERN_LEN 256

int match(const char *pattern, const char *str) {
    while (*pattern && *str) {
        if (*pattern == '*') {
            
            while (*pattern == '*') pattern++; 
            if (!*pattern) return 1;

            while (*str) {
                if (match(pattern, str)) return 1; 
                str++;
            }
            return 0;
        } else if (*pattern == '?' || *pattern == *str) {
            pattern++;
            str++;
        } else {
            return 0;  
        }
    }

    while (*pattern == '*') pattern++;  
    return !*pattern && !*str;  
}

int main() {
    char pattern[MAX_PATTERN_LEN];
    scanf("%s", pattern);

    DIR *dir = opendir(".");

    struct dirent *entry;
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
