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
#include <semaphore.h>

#define PATH_MAX 4096
#define BUF_SIZE 4096
#define MAX_RETRIES 5
#define RETRY_DELAY 1
#define MAX_THREADS 100

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;
sem_t thread_semaphore;

typedef struct {
    pthread_t *threads;
    size_t count;
    size_t capacity;
} ThreadPool;

ThreadPool thread_pool;

void init_thread_pool(ThreadPool *pool, size_t capacity) {
    pool->threads = malloc(capacity * sizeof(pthread_t));
    pool->count = 0;
    pool->capacity = capacity;
}

void add_thread_to_pool(ThreadPool *pool, pthread_t thread) {
    if (pool->count < pool->capacity) {
        pool->threads[pool->count++] = thread;
    } else {
        fprintf(stderr, "Thread pool is full\n");
    }
}

void wait_for_threads(ThreadPool *pool) {
    for (size_t i = 0; i < pool->count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

void free_thread_pool(ThreadPool *pool) {
    free(pool->threads);
}

int safe_open(const char* path, int flags, mode_t mode) {
    int fd;
    pthread_mutex_lock(&fileopen_mutex);
    while ((fd = open(path, flags, mode)) == -1) {
        if (errno == EMFILE) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        } else {
            perror("open");
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
            perror("opendir");
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
    free(arg);

    int src_fd = safe_open(src_path, O_RDONLY, 0);
    if (src_fd == -1) {
        sem_post(&thread_semaphore);
        return NULL;
    }

    int dst_fd = safe_open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        close(src_fd);
        sem_post(&thread_semaphore);
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
    pthread_cond_signal(&fileopen_cond);
    sem_post(&thread_semaphore);
    return NULL;
}

int create_thread_with_retry(pthread_t* thread, void* (*start_routine)(void*), void* arg) {
    int retries = 0;
    while (pthread_create(thread, NULL, start_routine, arg) != 0) {
        if (errno == EAGAIN && retries < MAX_RETRIES) {
            retries++;
            sleep(RETRY_DELAY);
        } else {
            perror("pthread_create");
            return -1;
        }
    }
    pthread_detach(*thread);
    return 0;
}

void* copy_directory_thread(void* arg) {
    const char* src_dir = ((char**)arg)[0];
    const char* dst_dir = ((char**)arg)[1];
    free(arg);

    DIR* dir = safe_opendir(src_dir);
    if (!dir) {
        sem_post(&thread_semaphore);
        return NULL;
    }

    long name_max = pathconf(src_dir, _PC_NAME_MAX);
    if (name_max == -1) {
        name_max = 255;
    }
    size_t len = sizeof(struct dirent) + name_max + 1;
    struct dirent* entry = alloca(len);

    struct dirent* result;
    while (readdir_r(dir, entry, &result) == 0 && result != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char* src_path = alloca(strlen(src_dir) + strlen(entry->d_name) + 2);
        char* dst_path = alloca(strlen(dst_dir) + strlen(entry->d_name) + 2);
        snprintf(src_path, PATH_MAX, "%s/%s", src_dir, entry->d_name);
        snprintf(dst_path, PATH_MAX, "%s/%s", dst_dir, entry->d_name);

        struct stat st;
        if (lstat(src_path, &st) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (mkdir(dst_path, 0755) == -1 && errno != EEXIST) {
                perror("mkdir");
                continue;
            }
            sem_wait(&thread_semaphore);
            pthread_t thread;
            char** args = malloc(2 * sizeof(char*));
            if (!args) {
                perror("malloc");
                sem_post(&thread_semaphore);
                continue;
            }
            args[0] = strdup(src_path);
            args[1] = strdup(dst_path);
            if (create_thread_with_retry(&thread, copy_directory_thread, args) != 0) {
                free(args[0]);
                free(args[1]);
                free(args);
                sem_post(&thread_semaphore);
            } else {
                add_thread_to_pool(&thread_pool, thread);
            }
        } else if (S_ISREG(st.st_mode)) {
            sem_wait(&thread_semaphore);
            pthread_t thread;
            char** args = malloc(2 * sizeof(char*));
            if (!args) {
                perror("malloc");
                sem_post(&thread_semaphore);
                continue;
            }
            args[0] = strdup(src_path);
            args[1] = strdup(dst_path);
            if (create_thread_with_retry(&thread, copy_file, args) != 0) {
                free(args[0]);
                free(args[1]);
                free(args);
                sem_post(&thread_semaphore);
            } else {
                add_thread_to_pool(&thread_pool, thread);
            }
        }
    }

    closedir(dir);
    pthread_cond_signal(&fileopen_cond);
    sem_post(&thread_semaphore);
    return NULL;
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

    sem_init(&thread_semaphore, 0, MAX_THREADS);

    init_thread_pool(&thread_pool, MAX_THREADS * 10);

    pthread_t thread;
    char** args = malloc(2 * sizeof(char*));
    if (!args) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    args[0] = strdup(src_dir);
    args[1] = strdup(dst_dir);
    if (create_thread_with_retry(&thread, copy_directory_thread, args) != 0) {
        free(args[0]);
        free(args[1]);
        free(args);
        return EXIT_FAILURE;
    } else {
        add_thread_to_pool(&thread_pool, thread);
    }

    wait_for_threads(&thread_pool);

    free_thread_pool(&thread_pool);
    sem_destroy(&thread_semaphore);

    return EXIT_SUCCESS;
}