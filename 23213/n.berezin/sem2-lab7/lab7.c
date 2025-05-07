#define _DEFAULT_SOURCE
#define _POSIX_PTHREAD_SEMANTICS

#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 1024

pthread_attr_t attr;

char *initial_from_path;
char *initial_to_path;

typedef struct thread_arg {
    int from_fd;
    int to_fd;
    char *relative_path;
} thread_arg;

/// Takes ownership of the argument
void copy_file(char *);

char *path_concat(char *path1, char *path2) {
    size_t path1_len = path1 ? strlen(path1) : 0;
    size_t path2_len = path2 ? strlen(path2) : 0;

    size_t result_len;
    if (path1_len == 0 && path2_len == 0) {
        result_len = 0;
    } else if (path1_len == 0) {
        result_len = path2_len;
    } else if (path2_len == 0) {
        result_len = path1_len;
    } else {
        result_len = path1_len + 1 + path2_len;
    }

    char *result = malloc(result_len + 1);
    if (result == NULL) {
        perror("Not enough memory");
        exit(1);
    }

    if (path1_len == 0 && path2_len == 0) {
        result[0] = '\0';
    } else if (path1_len == 0) {
        memcpy(result, path2, path2_len);
        result[path2_len] = '\0';
    } else if (path2_len == 0) {
        memcpy(result, path1, path1_len);
        result[path1_len] = '\0';
    } else {
        memcpy(result, path1, path1_len);
        result[path1_len] = '/';
        memcpy(&result[path1_len + 1], path2, path2_len);
        result[path1_len + path2_len + 1] = '\0';
    }
    return result;
}

char *from_path(char *relative_path) {
    return path_concat(initial_from_path, relative_path);
}

char *to_path(char *relative_path) {
    return path_concat(initial_to_path, relative_path);
}

void *copy_reg(void *void_arg) {
    thread_arg *arg = (thread_arg *)void_arg;

    char buf[BUF_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(arg->from_fd, buf, BUF_SIZE)) > 0) {
        if (write(arg->to_fd, buf, bytes_read) < 0) {
            char *to = to_path(arg->relative_path);
            fprintf(stderr, "Failed to write to file %s: %s\n", to, strerror(errno));
            free(to);
            break;
        }
    }

    if (bytes_read < 0) {
        char *from = from_path(arg->relative_path);
        fprintf(stderr, "Failed to read file %s: %s\n", from, strerror(errno));
        free(from);
    }

    close(arg->from_fd);
    close(arg->to_fd);
    if (arg->relative_path != NULL) {
        free(arg->relative_path);
    }
    free(arg);


    return NULL;
}

/// Argument is a path, relative to the `from` path.
/// Collects all files in the directory, into a NULL-terminated array of strings.
/// The array and all it's contents must be freed.
char **collect_directory(DIR *dir, char *relative_directory_path) {
    char *from_directory_path = from_path(relative_directory_path);

    long name_max = pathconf(from_directory_path, _PC_NAME_MAX);
    struct dirent *entry = malloc(sizeof(struct dirent) + name_max + 1);
    if (entry == NULL) {
        perror("Not enough memory");
        exit(1);
    }

    char **files = malloc(sizeof(char *) * 4);
    if (files == NULL) {
        perror("Not enough memory");
        exit(1);
    }
    files[0] = NULL;
    size_t files_len = 0;
    size_t files_cap = 4;

    while (1) {
        struct dirent *result;
        int dir_read_res = readdir_r(dir, entry, &result);
        if (dir_read_res != 0) {
            fprintf(stderr, "Failed to read directory %s: %s\n", from_directory_path, strerror(dir_read_res));
            break;
        }
        if (result == NULL) {
            break;
        }

        if (strcmp(result->d_name, ".") == 0 || strcmp(result->d_name, "..") == 0) {
            continue;
        }

        char *file_path = path_concat(relative_directory_path, result->d_name);

        if (files_len + 1 == files_cap) {
            files_cap *= 2;
            files = realloc(files, sizeof(char *) * files_cap);
        }
        files[files_len] = file_path;
        files[files_len + 1] = NULL;
        files_len++;
    }

    free(entry);
    free(from_directory_path);

    return files;
}

/// Takes ownership of the thread_arg
void *copy_dir(void *void_arg) {
    thread_arg *arg = (thread_arg *)void_arg;

    DIR *dir = fdopendir(arg->from_fd);    
    if (dir == NULL) {
        char *from = from_path(arg->relative_path);
        fprintf(stderr, "Failed to open directory stream for %s: %s\n", from, strerror(errno));
        free(from);
        if (arg->relative_path != NULL) {
            free(arg->relative_path);
        }
        close(arg->from_fd);
        free(arg);
        return NULL;
    }
    
    char **files = collect_directory(dir, arg->relative_path);
    closedir(dir);

    for (int i = 0; files[i] != NULL; i++) {
        copy_file(files[i]);
    }

    free(files);
    if (arg->relative_path != NULL) {
        free(arg->relative_path);
    }
    free(arg);

    return NULL;
}

/// Takes ownership of the argument
void copy_file(char *relative_path) {
    char *from = from_path(relative_path);
    
    int from_fd;
    while ((from_fd = open(from, O_RDONLY)) < 0) {
        if (errno == EMFILE) {
            sleep(1);
            continue;
        }
        fprintf(stderr, "Failed to open %s: %s\n", from, strerror(errno));
        free(from);
        if (relative_path != NULL) {
            free(relative_path);
        }
        return;
    }

    struct stat st;
    if (fstat(from_fd, &st) < 0) {
        fprintf(stderr, "Failed to fetch file type of %s: %s\n", from, strerror(errno));
        close(from_fd);
        free(from);
        if (relative_path != NULL) {
            free(relative_path);
        }
        return;
    }

    char *to = to_path(relative_path);
    void *(*thread)(void *) = NULL;

    thread_arg *arg = malloc(sizeof(thread_arg));
    if (arg == NULL) {
        perror("Not enough memory");
        exit(1);
    }
    arg->from_fd = from_fd;
    arg->relative_path = relative_path;
    arg->to_fd = -1;

    if (S_ISREG(st.st_mode)) {
        while ((arg->to_fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode)) < 0) {
            if (errno == EMFILE) {
                sleep(1);
                continue;
            }
            fprintf(stderr, "Failed to open %s: %s\n", to, strerror(errno));
            break;
        }

        if (arg->to_fd >= 0) {
            thread = copy_reg;
        }
    } else if (S_ISDIR(st.st_mode)) {
        if (mkdir(to, 0755) < 0) {
            fprintf(stderr, "Failed to create directory %s: %s\n", to, strerror(errno));
        } else {
            thread = copy_dir;
        }
    } else {
        fprintf(stderr, "Unsupported file type of %s\n", from);
    }

    if (thread != NULL) {
        pthread_t tid;
        // printf("Creating thread for %s\n", arg->relative_path);
        int created = pthread_create(&tid, &attr, thread, arg);
        if (created != 0) {
            fprintf(stderr, "Failed to create thread. %s not copied: %s\n", from, strerror(created));
            thread = NULL;
        }
    }

    if (thread == NULL) {
        if (arg->to_fd >= 0) {
            close(arg->to_fd);
        }
        if (relative_path != NULL) {
            free(relative_path);
        }
        free(arg);
        close(from_fd);
    }
    free(to);
    free(from);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: lab7 <from> <to>\n");
        return 1;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    initial_from_path = argv[1];
    initial_to_path = argv[2];

    size_t from_path_len = strlen(initial_from_path);
    size_t to_path_len = strlen(initial_to_path);
    if (initial_from_path[from_path_len - 1] == '/') {
        initial_from_path[from_path_len - 1] = '\0';
    }
    if (initial_to_path[to_path_len - 1] == '/') {
        initial_to_path[to_path_len - 1] = '\0';
    }

    copy_file(NULL);
    
    pthread_exit(NULL);
}
