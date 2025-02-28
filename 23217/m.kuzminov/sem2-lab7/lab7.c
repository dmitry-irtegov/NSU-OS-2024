#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

typedef struct paths_thread {
    char src_path[PATH_MAX + 1];
    char dest_path[PATH_MAX + 1];
} paths_thread;

DIR* open_dir(char* dir_name) {
    DIR* main_dir;
    while ((main_dir = opendir(dir_name)) == NULL && errno == EMFILE) {
        sleep(1);
    }
    if (main_dir == NULL) {
        fprintf(stderr, "    %s\n", dir_name);
        perror("opendir");
        exit(1);
    }
    return main_dir;
}

int open_file(char* file_name, int flags) {
    int fd;
    while ((fd = open(file_name, flags, 0777)) == -1 && errno == EMFILE) {
        sleep(1);
    }
    if (fd == -1) {
        fprintf(stderr, "    %s\n", file_name);
        perror("open file");
        exit(1);
    }
    return fd;
}

void* copy_file(void* paths_void) {
    paths_thread* paths = (paths_thread*) paths_void;
    char buf[BUFSIZ];

    int src_fd = open_file(paths->src_path, O_RDONLY);
    int dest_fd = open_file(paths->dest_path, O_RDWR|O_CREAT|O_TRUNC);

    ssize_t size;
    while ((size = read(src_fd, buf, sizeof(buf))) > 0) {
        char* current_pos = buf;
        ssize_t remaining = size;
        
        while (remaining > 0) {
            ssize_t written_now = write(dest_fd, current_pos, remaining);
            if (written_now < 0) {
                perror("write");
                exit(1);
            }
            
            current_pos += remaining;
            remaining -= size;
        }
    }

    if (size < 0) {
        perror("read");
        exit(1);
    }
    close(src_fd);
    close(dest_fd);
    free(paths);
    return NULL;
}

void* in_dir(void* paths_void) {
    paths_thread* paths = (paths_thread*) paths_void;
    DIR* dir_src = open_dir(paths->src_path);

    struct dirent* entry = malloc(sizeof(struct dirent)+pathconf(paths->src_path, _PC_NAME_MAX));
    struct dirent* result;
    struct stat about_file;

    while(readdir_r(dir_src, entry, &result) == 0 && result != NULL) {
        char* full_path = (char*)malloc(PATH_MAX);
        char* full_dest_path = (char*)malloc(PATH_MAX);
        if(full_path == NULL || full_dest_path == NULL) {
            perror("malloc");
            exit(1);
        }

        strcpy(full_path, paths->src_path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        
        strcpy(full_dest_path, paths->dest_path);
        strcat(full_dest_path, "/");
        strcat(full_dest_path, entry->d_name);

        if (stat(full_path, &about_file) == -1) {
            perror("stat");
            exit(1);
        }

        mkdir(paths->dest_path, 0777);

        if (S_ISDIR(about_file.st_mode) && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            
            // printf("%s\n", full_path);
            
            pthread_t thread;
            paths_thread* new_paths = malloc(sizeof(paths_thread));

            memcpy(new_paths->src_path, full_path, strlen(full_path) + 1);
            memcpy(new_paths->dest_path, full_dest_path, strlen(full_dest_path) + 1);

            int res = pthread_create(&thread, NULL, in_dir, (void*) new_paths);
            if (res != 0) {
                perror("pthread_create");
                exit(1);
            }
            pthread_detach(thread);
        } else if (S_ISREG(about_file.st_mode)) {
            pthread_t thread;
            paths_thread* new_paths = malloc(sizeof(paths_thread));

            memcpy(new_paths->src_path, full_path, strlen(full_path) + 1);
            memcpy(new_paths->dest_path, full_dest_path, strlen(full_dest_path) + 1);

            int res = pthread_create(&thread, NULL, copy_file, (void*) new_paths);
            if (res != 0) {
                perror("pthread_create");
                exit(1);
            }
            pthread_detach(thread);

        }
        free(full_path);
        free(full_dest_path);
    }

    closedir(dir_src);
    free(paths);
    free(entry);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "You need 2 arguement: src and dest\n");
        exit(1);
    }

    paths_thread* paths = malloc(sizeof(paths_thread));
    memcpy(paths->src_path, argv[1], strlen(argv[1]) + 1);
    memcpy(paths->dest_path, argv[2], strlen(argv[2]) + 1);

    pthread_t thread;
    int res = pthread_create(&thread, NULL, in_dir, (void*) paths);
    if (res != 0) {
        perror("pthread_create");
        exit(1);
    }
    pthread_detach(thread);

    pthread_exit(NULL);

    return 0;
}
