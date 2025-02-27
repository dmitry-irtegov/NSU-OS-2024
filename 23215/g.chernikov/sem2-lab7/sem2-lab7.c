#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <limits.h>

typedef struct {
    char* source;
    char* target;
}params;

void* copy_file(void* param) {
    char* source = (char*)((params*)param)->source;
    char* target = (char*)((params*)param)->target;

    printf("Copying file from %s to %s\n", source, target);

    int src_fd = open(source, O_RDONLY);
    if (src_fd == -1) {
        char buf[256];
        strerror_r(errno, buf, sizeof(buf));
        fprintf(stderr, "Failed to open source file: %s\n", buf);
        return 0;
    }

    int trgt_fd = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (trgt_fd == -1) {
        char buf[256];
        strerror_r(errno, buf, sizeof(buf));
        fprintf(stderr, "Failed to open target file: %s\n", buf);
        return 0;
    }

    char buffer[2048];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) != 0) {
        if (bytes_read == -1) {
            char buf[256];
            strerror_r(errno, buf, sizeof(buf));
            fprintf(stderr, "Failed to read from source file: %s\n", buf);
            break;
        }

        bytes_written = write(trgt_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            char buf[256];
            strerror_r(errno, buf, sizeof(buf));
            fprintf(stderr, "Failed to write to target file: %s\n", buf);
            break;
        }
    }
    close(src_fd);
    close(trgt_fd);
    free(((params*)param)->source);
    free(((params*)param)->target);
    free(param);
    return 0;
}


void* directory_explorer(void* parameters) {

    char* source = (char*)((params*)parameters)->source;
    char* target = (char*)((params*)parameters)->target;

    DIR* dir = opendir(source);
    if (!dir) {
        perror("Failed to open directory");
        return NULL;
    }

    long max_name = pathconf(source, _PC_NAME_MAX);
    long max_path = pathconf(source, _PC_PATH_MAX);

    struct dirent* entry = malloc(sizeof(struct dirent) + max_name + 1);
    struct dirent* result = NULL;

    while(1) {

        if (readdir_r(dir, entry, &result) != 0 || result == NULL) {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source_pwd[max_path];
        char target_pwd[max_path];

        snprintf(source_pwd, max_path, "%s/%s", source, entry->d_name);
        snprintf(target_pwd, max_path, "%s/%s", target, entry->d_name);

        struct stat st;
        if (stat(source_pwd, &st) != 0) {
            char buf[256];
            strerror_r(errno, buf, sizeof(buf));
            fprintf(stderr, "Failed to stat file: %s\n", buf);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {

            if (mkdir(target_pwd, 0777) != 0 && errno != EEXIST) {
                char buf[256];
                strerror_r(errno, buf, sizeof(buf));
                fprintf(stderr, "Failed to create directory: %s\n", buf);
                continue;
            }
            pthread_t thread;
            params* param = malloc(sizeof(params));
            param->source = strdup(source_pwd);
            param->target = strdup(target_pwd);
            pthread_create(&thread, NULL, directory_explorer, param);
            pthread_detach(thread);

        } else if (S_ISREG(st.st_mode)) {
            pthread_t thread;
            params* param = malloc(sizeof(params));
            param->source = strdup(source_pwd);
            param->target = strdup(target_pwd);
            pthread_create(&thread, NULL, copy_file, param);
            pthread_detach(thread);
        }
    }
    free(result);
    free(((params*)parameters)->source);
    free(((params*)parameters)->target);
    free(parameters);
    closedir(dir);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "%s + Source + Target", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (stat(argv[1], &st) != 0) {
        fprintf(stderr, "Source directory doesnt exist\n");
        exit(EXIT_FAILURE);

    } else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Source directory is not a directory)\n");
        exit(EXIT_FAILURE);
    }

    if (mkdir(argv[2], 0777) != 0 && errno != EEXIST) {
        fprintf(stderr, "Cant create a directory\n");
        exit(EXIT_FAILURE);
    }

    params* param = malloc(sizeof(params));
    param->source = strdup(argv[1]);
    param->target = strdup(argv[2]);

    directory_explorer(param);
    pthread_exit(0);
}