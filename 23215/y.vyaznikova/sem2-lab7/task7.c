#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define BUF_SIZE 4096

void* copy_file(void* arg) {
    const char* src_path = ((char**)arg)[0];
    const char* dst_path = ((char**)arg)[1];
    free(arg);

    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        perror("open src");
        return NULL;
    }

    int dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        perror("open dst");
        close(src_fd);
        return NULL;
    }

    char buf[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buf, BUF_SIZE)) > 0) {
        bytes_written = write(dst_fd, buf, bytes_read);
        if (bytes_written != bytes_read) {
            perror("write");
            break;
        }
    }

    close(src_fd);
    close(dst_fd);
    return NULL;
}

void copy_directory(const char* src_dir, const char* dst_dir) {
    DIR* dir = opendir(src_dir);
    if (!dir) {
        perror("opendir");
        return;
    }

    long name_max = pathconf(src_dir, _PC_NAME_MAX);
    if (name_max == -1) {
        name_max = 255;
    }
    size_t len = sizeof(struct dirent) + name_max + 1;
    struct dirent* entry = malloc(len);
    if (!entry) {
        perror("malloc");
        closedir(dir);
        return;
    }

    struct dirent* result;
    while (readdir_r(dir, entry, &result) == 0 && result != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[PATH_MAX];
        char dst_path[PATH_MAX];
        snprintf(src_path, PATH_MAX, "%s/%s", src_dir, entry->d_name);
        snprintf(dst_path, PATH_MAX, "%s/%s", dst_dir, entry->d_name);

        struct stat st;
        if (stat(src_path, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (mkdir(dst_path, 0755) == -1 && errno != EEXIST) {
                perror("mkdir");
                continue;
            }
            copy_directory(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            pthread_t thread;
            char** args = malloc(2 * sizeof(char*));
            if (!args) {
                perror("malloc");
                continue;
            }
            args[0] = strdup(src_path);
            args[1] = strdup(dst_path);
            if (pthread_create(&thread, NULL, copy_file, args) != 0) {
                perror("pthread_create");
                free(args[0]);
                free(args[1]);
                free(args);
            } else {
                pthread_detach(thread);
            }
        }
    }

    free(entry);
    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source directory> <destination directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* src_dir = argv[1];
    const char* dst_dir = argv[2];

    struct stat st;
    if (stat(src_dir, &st) == -1 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Source is not a directory\n");
        return EXIT_FAILURE;
    }

    if (mkdir(dst_dir, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        return EXIT_FAILURE;
    }

    copy_directory(src_dir, dst_dir);

    return EXIT_SUCCESS;
}