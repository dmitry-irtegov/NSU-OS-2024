#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    printf("Real user id: %u\n", getuid());
    printf("Effective user id: %u\n", geteuid());

    FILE *file = fopen("text.txt", "r+");
    if (file == NULL)
    {
        perror("File can not be opened");
    }
    else
    {
        fclose(file);
    }

    if (setuid(getuid()) == -1)
    {
        perror("Failed with setuid");
        exit(EXIT_FAILURE);
    }

    printf("Real user id: %u\n", getuid());
    printf("Effective user id: %u\n", geteuid());
    file = fopen("text.txt", "r+");
    if (file == NULL)
    {
        perror("File can not be opened");
    }
    else
    {
        fclose(file);
    }
    exit(EXIT_SUCCESS);
}