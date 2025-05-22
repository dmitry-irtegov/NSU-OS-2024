#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>

#define BUF_SIZE 8192

typedef struct {
    char src[PATH_MAX];
    char dst[PATH_MAX];
} copy_args_t;

void *copy_file_thread(void *arg) {
    copy_args_t *args = (copy_args_t *)arg;

    int in_fd, out_fd;
    char buf[BUF_SIZE];
    ssize_t bytes;

    while ((in_fd = open(args->src, O_RDONLY)) == -1 && errno == EMFILE) {
        sleep(1);
    }
    if (in_fd == -1) {
        perror("open source");
        free(args);
        return NULL;
    }

    while ((out_fd = open(args->dst, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1 && errno == EMFILE) {
        sleep(1);
    }
    if (out_fd == -1) {
        perror("open destination");
        close(in_fd);
        free(args);
        return NULL;
    }

    while ((bytes = read(in_fd, buf, BUF_SIZE)) > 0) {
        if (write(out_fd, buf, bytes) != bytes) {
            perror("write");
            break;
        }
    }

    close(in_fd);
    close(out_fd);
    free(args);
    return NULL;
}

void *copy_dir_thread(void *arg);

void spawn_copy_dir(const char *src, const char *dst) {
    pthread_t tid;
    copy_args_t *args = malloc(sizeof(copy_args_t));
    if (!args) {
        perror("malloc");
        return;
    }
    strncpy(args->src, src, PATH_MAX);
    strncpy(args->dst, dst, PATH_MAX);

    pthread_create(&tid, NULL, copy_dir_thread, args);
    pthread_detach(tid);
}

void *copy_dir_thread(void *arg) {
    copy_args_t *args = (copy_args_t *)arg;
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    pthread_t threads[1024];
    int thread_count = 0;

    mkdir(args->dst, 0755);

    long name_max = pathconf(args->src, _PC_NAME_MAX);
    size_t buf_size = sizeof(struct dirent) + name_max + 1;
    struct dirent *entry_buf = malloc(buf_size);
    if (!entry_buf) {
        perror("malloc dirent");
        free(args);
        return NULL;
    }

    while ((dir = opendir(args->src)) == NULL && errno == EMFILE) {
        sleep(1);
    }
    if (!dir) {
        perror("opendir");
        free(entry_buf);
        free(args);
        return NULL;
    }

    while (readdir_r(dir, entry_buf, &entry) == 0 && entry != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[PATH_MAX], dst_path[PATH_MAX];
        snprintf(src_path, PATH_MAX, "%s/%s", args->src, entry->d_name);
        snprintf(dst_path, PATH_MAX, "%s/%s", args->dst, entry->d_name);

        if (stat(src_path, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            spawn_copy_dir(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            copy_args_t *fargs = malloc(sizeof(copy_args_t));
            if (!fargs) {
                perror("malloc");
                continue;
            }
            strncpy(fargs->src, src_path, PATH_MAX);
            strncpy(fargs->dst, dst_path, PATH_MAX);
            pthread_create(&threads[thread_count++], NULL, copy_file_thread, fargs);
        }
    }

    for (int i = 0; i < thread_count; ++i)
        pthread_join(threads[i], NULL);

    closedir(dir);
    free(entry_buf);
    free(args);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_dir> <dest_dir>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct stat st;
    if (stat(argv[1], &st) == -1 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Source must be a directory\n");
        return EXIT_FAILURE;
    }

    mkdir(argv[2], 0755);

    copy_args_t *args = malloc(sizeof(copy_args_t));
    if (!args) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    strncpy(args->src, argv[1], PATH_MAX);
    strncpy(args->dst, argv[2], PATH_MAX);

    pthread_t main_thread;
    pthread_create(&main_thread, NULL, copy_dir_thread, args);
    pthread_join(main_thread, NULL);

    return EXIT_SUCCESS;
}
