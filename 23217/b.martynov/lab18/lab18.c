#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

char* shortFileName(char* fileName)
{
    char* name = fileName;
    for (int ptr = 0; name[ptr] != '\0'; ptr++) {
        if (name[ptr] == '/')
        {
            name = name + ptr + 1;
            ptr = -1;
        }
    }
    return name;
}

void func(char* fileName)
{
    struct stat buf;

    if (stat(fileName, &buf) == -1) {
        perror("stat was broken");
        return;
    }


    char mask[] = "?---___---";
    mask[0] = S_ISDIR(buf.st_mode) ? 'd' : (S_ISREG(buf.st_mode) ? '-' : '?');

    mask[1] = S_IRUSR & buf.st_mode ? 'r' : '-';
    mask[2] = S_IWUSR & buf.st_mode ? 'w' : '-';
    mask[3] = S_IXUSR & buf.st_mode ? 'x' : '-';

    mask[4] = S_IRGRP & buf.st_mode ? 'r' : '-';
    mask[5] = S_IWGRP & buf.st_mode ? 'w' : '-';
    mask[6] = S_IXGRP & buf.st_mode ? 'x' : '-';

    mask[7] = S_IROTH & buf.st_mode ? 'r' : '-';
    mask[8] = S_IWOTH & buf.st_mode ? 'w' : '-';
    mask[9] = S_IXOTH & buf.st_mode ? 'x' : '-';


    uid_t uid = buf.st_uid;
    gid_t gid = buf.st_gid;

    char* empty = "-";
    struct passwd* aboutUser;
    char* userName;
    if (((aboutUser = getpwuid(uid)) == NULL) || ((userName = aboutUser->pw_name) == NULL))
    {
        perror("Can't get user name");
        userName = empty;
    }
    struct group* aboutGroup;
    char* groupName;
    if (((aboutGroup = getgrgid(gid)) == NULL) || ((groupName = aboutGroup->gr_name) == NULL))
    {
        perror("Can't get group name");
        groupName = empty;
    }


    off_t fileSize = S_ISREG(buf.st_mode) ? buf.st_size : -1;

    time_t fileMTime = buf.st_mtime;
    char* date = ctime(&fileMTime);

    char* sFileName = shortFileName(fileName);

    if (fileSize == (off_t)-1) {
        printf("%s %s %s %20s %s %s\n", mask, userName, groupName, empty, date, sFileName);
    }
    else  {
#if defined(_LP64) || _FILE_OFFSET_BITS == 32
        printf("%s %s %s %20ld %s %s\n", mask, userName, groupName, fileSize, date, sFileName);
#elif _FILE_OFFSET_BITS == 64
        printf("%s %s %s %20lld %s %s\n", mask, userName, groupName, fileSize, date, sFileName);
#endif
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        func(".");
    }
    else {
        for (int i = 1; i < argc; i++) {
            func(argv[i]);
        }
    }
}
