#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s filein \n", argv[0]);
        return 1;
    }
    char* fileName = argv[1];
    int Descriptor = open(fileName, O_RDONLY);
    if(Descriptor == -1) {
        perror("Couldn`t open file");
        return 1;
    }
    int check_errno = errno;
    FILE * pipe; 
    if ((pipe = popen("tr 'a-z' 'A-Z'", "w")) == NULL) {
        if(errno != check_errno) {
            perror("Couldn`t open pipe");
            return 1;
        }
        fprintf(stderr, "Couldn`t open pipe");
        return 1;
    }
    char buf[BUFSIZ];
    int n;
    while ((n = read(Descriptor, buf, BUFSIZ)) > 0)
        write(fileno(pipe), buf, n);
    pclose(pipe);
    return 0;
}
