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
#include <semaphore.h>
#include <sys/resource.h>


typedef struct {
    char* source;
    char* target;
} params;

sem_t thread_semaphore;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t file_cond = PTHREAD_COND_INITIALIZER;

void* copy_file(void* param) {

    
    char* source = (char*)((params*)param)->source;
    char* target = (char*)((params*)param)->target;
    int src_fd = -1;
    int trgt_fd = -1;
    // printf("Copying file from %s to %s\n", source, target);


    pthread_mutex_lock(&file_mutex);
    while(src_fd < 0 || trgt_fd < 0) {
        src_fd = open(source, O_RDONLY);
        if (src_fd < 0 && errno != EMFILE) {
            char buf[256];
            strerror_r(errno, buf, sizeof(buf));
            fprintf(stderr, "Failed to open source file: %s\n", buf);
            pthread_mutex_unlock(&file_mutex);
            return 0;
        }

        if (src_fd > 0) {
            trgt_fd = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if (trgt_fd < 0 && errno != EMFILE) {
                char buf[256];
                strerror_r(errno, buf, sizeof(buf));
                fprintf(stderr, "Failed to open target file: %s\n", buf);
                close(src_fd);
                pthread_mutex_unlock(&file_mutex);
                pthread_cond_signal(&file_cond);
                return 0;
            }
        }
        if (src_fd < 0 || trgt_fd < 0) {
            if (src_fd > 0) {
                close(src_fd);
                pthread_cond_signal(&file_cond);
            }
            printf("waiting\n");
            pthread_cond_wait(&file_cond, &file_mutex);
        }
    }
    pthread_mutex_unlock(&file_mutex);

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
    pthread_cond_signal(&file_cond);
    free(((params*)param)->source);
    free(((params*)param)->target);
    free(param);

    sem_post(&thread_semaphore);

    pthread_exit(0);
}


void* directory_explorer(void* parameters) {

    
    char* source = (char*)((params*)parameters)->source;
    char* target = (char*)((params*)parameters)->target;
    DIR* dir = NULL;

    while(dir == NULL) {
        dir = opendir(source);
        if (!dir && errno != EMFILE) {
            pthread_mutex_unlock(&file_mutex);
            perror("Failed to open directory");
            return 0;
        } else if(!dir && errno == EMFILE){
            pthread_cond_wait(&file_cond, &file_mutex);
        }
    }

    long max_name = pathconf(source, _PC_NAME_MAX);
    long max_path = pathconf(source, _PC_PATH_MAX);

    struct dirent* entry = malloc(sizeof(struct dirent) + max_name + 1);
    struct dirent* result = NULL;

    while (1) {
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
            sem_wait(&thread_semaphore);
            int result;
            if ((result = pthread_create(&thread, NULL, copy_file, param)) != 0) {
                fprintf(stderr, "Error creating thread: %s\n", strerror(result));
            }
            pthread_detach(thread);

        } else if (S_ISREG(st.st_mode)) {
            pthread_t thread;
            params* param = malloc(sizeof(params));
            param->source = strdup(source_pwd);
            param->target = strdup(target_pwd);
            sem_wait(&thread_semaphore);
            int result;
            if ((result = pthread_create(&thread, NULL, copy_file, param)) != 0) {
                fprintf(stderr, "Error creating thread: %s\n", strerror(result));
            }
            pthread_detach(thread);
        }
    }

    free(result);
    free(((params*)parameters)->source);
    free(((params*)parameters)->target);
    free(parameters);

    closedir(dir);
    pthread_cond_signal(&file_cond);
    sem_post(&thread_semaphore);
    pthread_exit(0);
}

int main(int argc, char** argv) {
    struct rlimit limit;
    if (getrlimit(RLIMIT_NPROC, &limit) == 0) {
        printf("Максимум потоков для пользователя: %llu\n", limit.rlim_cur);
    } else {
        perror("Ошибка получения RLIMIT_NPROC");
    }

    sem_init(&thread_semaphore, 0, limit.rlim_cur);

    if (argc != 3) {
        fprintf(stderr, "%s + Source + Target\n", argv[0]);
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