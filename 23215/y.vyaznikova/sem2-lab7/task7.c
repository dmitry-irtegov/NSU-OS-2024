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

#define BUF_SIZE 4096
#define MAX_RETRIES 5
#define RETRY_DELAY 1

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

int safe_open(const char* path, int flags, mode_t mode) {
    int fd;
    pthread_mutex_lock(&fileopen_mutex);
    while ((fd = open(path, flags, mode)) == -1) {
        if (errno == EMFILE) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        } else {
            fprintf(stderr, "Error: open failed for '%s': %s\n", path, strerror(errno));
            pthread_mutex_unlock(&fileopen_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&fileopen_mutex);
    return fd;
}

DIR* safe_opendir(const char* path) {
    DIR* dir;
    pthread_mutex_lock(&fileopen_mutex);
    while ((dir = opendir(path)) == NULL) {
        if (errno == EMFILE) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        } else {
            fprintf(stderr, "Error: opendir failed for '%s': %s\n", path, strerror(errno));
            pthread_mutex_unlock(&fileopen_mutex);
            return NULL;
        }
    }
    pthread_mutex_unlock(&fileopen_mutex);
    return dir;
}

void* copy_file(void* arg) {
    const char* src_path = ((char**)arg)[0];
    const char* dst_path = ((char**)arg)[1];

    int src_fd = safe_open(src_path, O_RDONLY, 0);
    if (src_fd == -1) {
        free(((char**)arg)[0]);
        free(((char**)arg)[1]);
        free(arg);
        pthread_exit(NULL);
    }

    int dst_fd = safe_open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        close(src_fd);
        free(((char**)arg)[0]);
        free(((char**)arg)[1]);
        free(arg);
        pthread_exit(NULL);
    }

    char buf[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buf, BUF_SIZE)) > 0) {
        bytes_written = write(dst_fd, buf, bytes_read);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error: write failed: %s\n", strerror(errno));
            break;
        }
    }

    close(src_fd);
    close(dst_fd);
    pthread_cond_signal(&fileopen_cond);

    free(((char**)arg)[0]);
    free(((char**)arg)[1]);
    free(arg);
    pthread_exit(NULL);
}

int create_thread_with_retry(pthread_t* thread, void* (*start_routine)(void*), void* arg) {
    int retries = 0;
    while (pthread_create(thread, NULL, start_routine, arg) != 0) {
        if (errno == EAGAIN && retries < MAX_RETRIES) {
            retries++;
            sleep(RETRY_DELAY);
        } else {
            fprintf(stderr, "Error: pthread_create failed: %s\n", strerror(errno));
            if (arg) {
                free(((char**)arg)[0]);
                free(((char**)arg)[1]);
                free(arg);
            }
            return -1;
        }
    }
    return 0;
}

void* copy_directory_thread(void* arg) {
    const char* src_dir = ((char**)arg)[0];
    const char* dst_dir = ((char**)arg)[1];

    DIR* dir = safe_opendir(src_dir);
    if (!dir) {
        free(((char**)arg)[0]);
        free(((char**)arg)[1]);
        free(arg);
        pthread_exit(NULL);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char* src_path = malloc(strlen(src_dir) + strlen(entry->d_name) + 2);
        char* dst_path = malloc(strlen(dst_dir) + strlen(entry->d_name) + 2);
        if (!src_path || !dst_path) {
            fprintf(stderr, "Error: malloc failed\n");
            free(src_path);
            free(dst_path);
            continue;
        }
        snprintf(src_path, PATH_MAX, "%s/%s", src_dir, entry->d_name);
        snprintf(dst_path, PATH_MAX, "%s/%s", dst_dir, entry->d_name);

        struct stat st;
        if (lstat(src_path, &st) == -1) {
            fprintf(stderr, "Error: lstat failed for '%s': %s\n", src_path, strerror(errno));
            free(src_path);
            free(dst_path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (mkdir(dst_path, 0755) == -1 && errno != EEXIST) {
                fprintf(stderr, "Error: mkdir failed for '%s': %s\n", dst_path, strerror(errno));
                free(src_path);
                free(dst_path);
                continue;
            }

            pthread_t thread;
            char** args = malloc(2 * sizeof(char*));
            if (!args) {
                fprintf(stderr, "Error: malloc failed\n");
                free(src_path);
                free(dst_path);
                continue;
            }
            args[0] = strdup(src_path);
            args[1] = strdup(dst_path);
            if (!args[0] || !args[1]) {
                fprintf(stderr, "Error: strdup failed\n");
                free(args[0]);
                free(args[1]);
                free(args);
                free(src_path);
                free(dst_path);
                continue;
            }
            if (create_thread_with_retry(&thread, copy_directory_thread, args) != 0) {
                free(args[0]);
                free(args[1]);
                free(args);
            } else {
                pthread_detach(thread);
            }
        } else if (S_ISREG(st.st_mode)) {
            pthread_t thread;
            char** args = malloc(2 * sizeof(char*));
            if (!args) {
                fprintf(stderr, "Error: malloc failed\n");
                free(src_path);
                free(dst_path);
                continue;
            }
            args[0] = strdup(src_path);
            args[1] = strdup(dst_path);
            if (!args[0] || !args[1]) {
                fprintf(stderr, "Error: strdup failed\n");
                free(args[0]);
                free(args[1]);
                free(args);
                free(src_path);
                free(dst_path);
                continue;
            }
            if (create_thread_with_retry(&thread, copy_file, args) != 0) {
                free(args[0]);
                free(args[1]);
                free(args);
            } else {
                pthread_detach(thread);
            }
        }

        free(src_path);
        free(dst_path);
    }

    closedir(dir);
    pthread_cond_signal(&fileopen_cond);

    free(((char**)arg)[0]);
    free(((char**)arg)[1]);
    free(arg);
    pthread_exit(NULL);
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
        fprintf(stderr, "Error: Source is not a directory\n");
        return EXIT_FAILURE;
    }

    if (mkdir(dst_dir, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "Error: mkdir failed for '%s': %s\n", dst_dir, strerror(errno));
        return EXIT_FAILURE;
    }

    pthread_t thread;
    char** args = malloc(2 * sizeof(char*));
    if (!args) {
        fprintf(stderr, "Error: malloc failed\n");
        return EXIT_FAILURE;
    }
    args[0] = strdup(src_dir);
    args[1] = strdup(dst_dir);
    if (!args[0] || !args[1]) {
        fprintf(stderr, "Error: strdup failed\n");
        free(args[0]);
        free(args[1]);
        free(args);
        return EXIT_FAILURE;
    }
    if (create_thread_with_retry(&thread, copy_directory_thread, args) != 0) {
        free(args[0]);
        free(args[1]);
        free(args);
        return EXIT_FAILURE;
    }

    pthread_exit(NULL);
}