#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

pthread_mutex_t file_open_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t file_open_cond = PTHREAD_COND_INITIALIZER;

typedef struct thread_arg {
    char *from_path;
    char *to_path;
} thread_arg;

typedef struct copy_file_arg {
    thread_arg thr_arg;
    bool succsessful_thread_creation;
    pthread_t tid;
} copy_file_arg;

void copy_file(copy_file_arg *arg);

char *path_concat(char *path1, char *path2) {
    size_t path1_len = strlen(path1);
    size_t path2_len = strlen(path2);

    size_t result_len = path1_len + 1 + path2_len + 1;

    char *result = malloc(result_len);
    if (result == NULL) {
        perror("Not enough memory");
        exit(1);
    }

    memcpy(result, path1, path1_len);
    result[path1_len] = '/';
    memcpy(&result[path1_len + 1], path2, path2_len);
    result[path1_len + 1 + path2_len] = '\0';

    return result;
}

void copy_reg(char *from_path, char *to_path, mode_t to_mode) {
    int from_fd, to_fd;

    pthread_mutex_lock(&file_open_mutex);
    while (1) {
        from_fd = open(from_path, O_RDONLY);
        if (from_fd < 0) {
            if (errno != EMFILE) {
                fprintf(stderr, "Failed to open %s: %s\n", from_path, strerror(errno));
                return;
            }
            pthread_cond_wait(&file_open_cond, &file_open_mutex);
            continue;
        }

        to_fd = open(to_path, O_WRONLY | O_CREAT | O_TRUNC, to_mode);
        if (to_fd < 0) {
            close(from_fd);
            if (errno != EMFILE) {
                fprintf(stderr, "Failed to open %s: %s\n", to_path, strerror(errno));
                return;
            }
            pthread_cond_wait(&file_open_cond, &file_open_mutex);
            continue;
        }

        break;
    }
    pthread_mutex_unlock(&file_open_mutex);

#define BUF_SIZE 1024

    char buf[BUF_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(from_fd, buf, BUF_SIZE)) > 0) {
        if (write(to_fd, buf, bytes_read) < 0) {
            fprintf(stderr, "Failed to write to file %s: %s\n", to_path, strerror(errno));
            break;
        }
    }

    if (bytes_read < 0) {
        fprintf(stderr, "Failed to read file %s: %s\n", from_path, strerror(errno));
    }

#undef BUF_SIZE

    close(from_fd);
    close(to_fd);
    pthread_cond_broadcast(&file_open_cond);
}

void copy_dir(char *from_path, char *to_path) {
    DIR *dir;
    pthread_mutex_lock(&file_open_mutex);
    while (1) {
        dir = opendir(from_path);
        if (dir == NULL) {
           if (errno != EMFILE) {
                fprintf(stderr, "Failed to open directory stream for %s: %s\n", from_path, strerror(errno));
                return;
            }
            pthread_cond_wait(&file_open_cond, &file_open_mutex);
            continue;
        }
        break;
    }
    pthread_mutex_unlock(&file_open_mutex);
    
    size_t files_cap = 4;
    size_t files_len = 0;
    copy_file_arg *files = malloc(sizeof(copy_file_arg)* files_cap);

    while (1) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (entry == NULL) {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char *new_from_path = path_concat(from_path, entry->d_name);
        char *new_to_path = path_concat(to_path, entry->d_name);

        if (files_len == files_cap) {
            files_cap *= 2;
            files = realloc(files, sizeof(copy_file_arg) * files_cap);
            if (files == NULL) {
                perror("Not enough memory");
                exit(1);
            }
        }

        files[files_len].thr_arg.from_path = new_from_path;
        files[files_len].thr_arg.to_path = new_to_path;

        files_len++;
    }
    if (errno != 0) {
        fprintf(stderr, "Failed to read directory %s: %s\n", from_path, strerror(errno));
    }
    closedir(dir);
    pthread_cond_broadcast(&file_open_cond);

    for (size_t i = 0; i < files_len; i++) {
        copy_file(&files[i]);
    }

    for (size_t i = 0; i < files_len; i++) {
        if (files[i].succsessful_thread_creation) {
            int err = pthread_join(files[i].tid, NULL);
            if (err != 0) {
                fprintf(stderr, "Failed to join thread for %s: %s\n", files[i].thr_arg.from_path, strerror(err));
            }
        }
        free(files[i].thr_arg.from_path);
        free(files[i].thr_arg.to_path);
    }
    free(files);
}

void *copy_file_thread_func(void *void_arg) {
    thread_arg *arg = (thread_arg *) void_arg;

    struct stat st;
    if (stat(arg->from_path, &st) < 0) {
        fprintf(stderr, "Failed to fetch file type of %s: %s\n", arg->from_path, strerror(errno));
        return NULL;
    }

    if (S_ISREG(st.st_mode)) {
        copy_reg(arg->from_path, arg->to_path, st.st_mode);
    } else if (S_ISDIR(st.st_mode)) {
        if (mkdir(arg->to_path, st.st_mode) < 0) {
            fprintf(stderr, "Failed to create directory %s: %s\n", arg->to_path, strerror(errno));
        } else {
            copy_dir(arg->from_path, arg->to_path);
        }
    } else {
        fprintf(stderr, "Unsupported file type of %s\n", arg->from_path);
    }

    return NULL;
}

void copy_file(copy_file_arg *arg) {    
    int created = pthread_create(&arg->tid, NULL, copy_file_thread_func, (void *)&arg->thr_arg);
    if (created != 0) {
        fprintf(stderr, "Failed to create thread. %s not copied: %s\n", arg->thr_arg.from_path, strerror(created));
        arg->succsessful_thread_creation = false;
    } else {
        arg->succsessful_thread_creation = true;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: lab7 <from> <to>\n");
        return 1;
    }

    char *from_path = argv[1];
    char *to_path = argv[2];

    size_t from_path_len = strlen(from_path);
    size_t to_path_len = strlen(to_path);

    if (from_path_len == 0 || to_path_len == 0) {
        printf("Usage: lab7 <from> <to>\n");
        return 1;
    }

    if (from_path[from_path_len - 1] == '/') {
        from_path[from_path_len - 1] = '\0';
    }
    if (to_path[to_path_len - 1] == '/') {
        to_path[to_path_len - 1] = '\0';
    }

    copy_file_arg arg;
    arg.thr_arg.from_path = from_path;
    arg.thr_arg.to_path = to_path;

    copy_file(&arg);

    if (arg.succsessful_thread_creation) {
        int err = pthread_join(arg.tid, NULL);
        if (err != 0) {
            fprintf(stderr, "Failed to join thread for %s: %s\n", arg.thr_arg.from_path, strerror(err));
        }
    }
}
