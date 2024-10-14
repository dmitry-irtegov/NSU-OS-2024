#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main() {
    printf("Реальный идентификатор пользователя: %d\n", getuid());
    printf("Эффективный идентификатор пользователя: %d\n", geteuid());

    FILE *file = fopen("file", "r");
    if (file == NULL) {
        perror("You have no rights to do this you bastard!");
    } else {
        printf("Welcome)\n");
        fclose(file);
    }

    if (seteuid(getuid()) == -1) {
        perror("seteuid");
        exit(EXIT_FAILURE);
    }

    printf("Реальный идентификатор пользователя: %d\n", getuid());
    printf("Эффективный идентификатор пользователя: %d\n", geteuid());

    file = fopen("file", "r");
    if (file == NULL) {
        perror("You have no rights to do this you bastard! I say it second time!");
    } else {
        printf("Welcome again)\n");
        fclose(file);
    }

    return 0;
}
