#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int open_file()
{
    FILE *file = fopen("testfile", "r");

    if (file == NULL)
    {
        perror("The file was not opened");
        return 1;
    }
    else
    {
        if (fclose(file) != 0)
        {
            perror("The file was not closed");
            return 2;
        }
        else
        {
            return 0;
        }
    }
}

int main()
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("uid = %d\neuid = %d\n", uid, euid);

    int open1 = open_file();
    if (open1 == 1)
    {
        exit(1);
    }
    if (open1 == 2)
    {
        exit(2);
    }

    if (setuid(uid) == -1)
    {
        perror("The effective user ID has not been set");
        exit(2);
    }

    uid_t updated_euid = geteuid();
    printf("uid = %d\neuid = %d\n", uid, updated_euid);

    int open2 = open_file();
    if (open2 == 1)
    {
        exit(3);
    }
    if (open2 == 2)
    {
        exit(4);
    }

    exit(0);
}