#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    FILE *open = fopen("test.txt", "a+");
    printf("Real ID - %d Effective ID - %d\n", getuid(), getegid());
    if(open== NULL)
    {
        perror("Error while opening a file");
    }
    fclose(open);
    setuid(getuid());
    printf("Real ID - %d Effective ID - %d\n", getuid(), getegid());
    FILE *open_new = fopen("test.txt", "a+");
    if(open== NULL)
    {
        perror("Error while opening a file");
    }
    fclose(open_new);
    return 0;
}