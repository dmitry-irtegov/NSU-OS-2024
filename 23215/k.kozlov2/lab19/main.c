#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>

int match(char *name, char *pattern);

int main() {
    char pattern[1000];
    char flag = 0;
    struct dirent *entry;

    DIR *dir = opendir(".");

    if (dir == NULL) {
        perror("can't open directory");
        return 1;
    }

    printf("Enter a pattern: ");
    (void)fgets(pattern, 1000, stdin);

    int len = strlen(pattern);

    if (pattern[len-1] == '\n')
        pattern[len-1] = '\0';
    else
        printf("\n");

    while((entry = readdir(dir)))
        if (match(entry->d_name, pattern)) {
            flag = 1;
            printf("%s\n", entry->d_name);
        }

    if (!flag)
        printf("%s\n", pattern);

    if (closedir(dir) == -1) {
        perror("can't close directory");
    }
    
    return 0;
}

int match(char *name, char *pattern) {
    int questionNumber;
    while(*pattern) {
        if (*pattern == '*') {
            questionNumber = 0;
            while (*pattern == '*' || *pattern == '?') {
                if (*pattern == '?')
                    questionNumber++;
                pattern++;
            }
            if (*pattern) {
                // not the end of the pattern.
                while (*name) {
                    if (*name == *pattern && questionNumber <= 0 && match(name, pattern))
                        return 1;
                    name++;
                    questionNumber--;
                }
                return 0;
            } else {
                //end of the pattern.
                while (*name) {
                    name++;
                    questionNumber--;
                }
                if (questionNumber <= 0)
                    return 1;
            }
        } else if (*pattern == '?') {
            if (*name) {
                pattern++;
                name++;
            } else
                return 0;
        } else {
            if (*pattern == *name) {
                pattern++;
                name++;
            } else
                return 0;
        }
    }

    if (*name)
        return 0;

    return 1;
}