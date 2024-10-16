#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

void print_user_ids()
{
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

void try_open_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
    }
    else
    {
        printf("File opened successfully\n");
        fclose(file);
    }
}

int main()
{
    const char *filename = "myfile.txt";

    printf("Before setuid:\n");
    print_user_ids();
    try_open_file(filename);

    if (setuid(getuid()) != 0)
    {
        perror("Error setting UID");
    }

    printf("\nAfter setuid:\n");
    print_user_ids();

    printf("Trying to open file again:\n");
    try_open_file(filename);

    return 0;
}