#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define MAX_THREADS 8
#define MAX_RETRIES 5

typedef enum { TASK_COPY_FILE, TASK_PROCESS_DIR, TASK_EXIT } TaskType;

typedef struct Task {
    TaskType type;
    char *source;
    char *dest;
    struct Task *next;
} Task;

typedef struct {
    Task *head;
    Task *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int active_workers;
    int pending_tasks;
} TaskQueue;

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

TaskQueue task_queue;

void queue_init() {
    task_queue.head = task_queue.tail = NULL;
    pthread_mutex_init(&task_queue.mutex, NULL);
    pthread_cond_init(&task_queue.cond, NULL);
    task_queue.active_workers = 0;
    task_queue.pending_tasks = 0;
}

void queue_push(TaskType type, const char *source, const char *dest) {
    Task *task = malloc(sizeof(Task));
    task->type = type;
    task->source = strdup(source);
    task->dest = strdup(dest);
    task->next = NULL;

    pthread_mutex_lock(&task_queue.mutex);
    if (task_queue.tail) {
        task_queue.tail->next = task;
        task_queue.tail = task;
    } else {
        task_queue.head = task_queue.tail = task;
    }
    task_queue.pending_tasks++;

    pthread_cond_broadcast(&task_queue.cond);
    pthread_mutex_unlock(&task_queue.mutex);
}

Task *queue_pop() {
    pthread_mutex_lock(&task_queue.mutex);
    while (!task_queue.head) {
        pthread_cond_wait(&task_queue.cond, &task_queue.mutex);
    }

    Task *task = NULL;
    if (task_queue.head) {
        task = task_queue.head;
        task_queue.head = task->next;
        if (!task_queue.head) task_queue.tail = NULL;
        task_queue.active_workers++;
        task_queue.pending_tasks--;
    }
    pthread_mutex_unlock(&task_queue.mutex);
    return task;
}

void task_free(Task *task) {
    if (task) {
        free(task->source);
        free(task->dest);
        free(task);
    }
}

void copy_file(const char *source, const char *dest) {
    int src_fd;
    int retries = 0;
    pthread_mutex_lock(&fileopen_mutex);
    while ((src_fd = open(source, O_RDONLY, 0)) == -1 && errno == EMFILE) {
        if (++retries > MAX_RETRIES) {
            pthread_mutex_unlock(&fileopen_mutex);
            fprintf(stderr, "Failed to open %s after %d retries\n", source, MAX_RETRIES);
            return;
        }
        pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
    }
    pthread_mutex_unlock(&fileopen_mutex);

    if (src_fd == -1) {
        perror("open source");
        return;
    }

    struct stat stat_buf;
    if (fstat(src_fd, &stat_buf) == -1) {
        perror("fstat");
        close(src_fd);
        pthread_cond_signal(&fileopen_cond);
        return;
    }

    int dest_fd;
    retries = 0;
    pthread_mutex_lock(&fileopen_mutex);
    while ((dest_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, stat_buf.st_mode)) == -1 && errno == EMFILE) {
        if (++retries > MAX_RETRIES) {
            pthread_mutex_unlock(&fileopen_mutex);
            close(src_fd);
            pthread_cond_signal(&fileopen_cond);
            fprintf(stderr, "Failed to open %s after %d retries\n", dest, MAX_RETRIES);
            return;
        }
        pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
    }
    pthread_mutex_unlock(&fileopen_mutex);

    if (dest_fd == -1) {
        perror("open dest");
        close(src_fd);
        pthread_cond_signal(&fileopen_cond);
        return;
    }

    char buffer[4096];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer)))) {
        if (bytes_read == -1) {
            perror("read");
            break;
        }

        char *ptr = buffer;
        while (bytes_read > 0) {
            bytes_written = write(dest_fd, ptr, bytes_read);
            if (bytes_written == -1) {
                perror("write");
                break;
            }
            bytes_read -= bytes_written;
            ptr += bytes_written;
        }
    }

    close(src_fd);
    close(dest_fd);
    pthread_cond_signal(&fileopen_cond);
}

void process_directory(const char *source, const char *dest) {
    DIR *dir = NULL;
    struct dirent *entry;
    long name_max;
    long path_max;
    size_t entry_size;
    struct dirent *entry_ptr;
    int retries = 0;

    struct stat src_stat;
    if (lstat(source, &src_stat) == -1) {
        perror("lstat source");
        return;
    }

    if (mkdir(dest, src_stat.st_mode) == -1 && errno != EEXIST) {
        perror("mkdir dest");
        return;
    }

    pthread_mutex_lock(&fileopen_mutex);
    while ((dir = opendir(source)) == NULL && errno == EMFILE) {
        if (++retries > MAX_RETRIES) {
            pthread_mutex_unlock(&fileopen_mutex);
            fprintf(stderr, "Failed to open directory %s after %d retries\n", source, MAX_RETRIES);
            return;
        }
        pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
    }
    pthread_mutex_unlock(&fileopen_mutex);

    if (!dir) {
        perror("opendir");
        return;
    }

    path_max = pathconf(source, _PC_PATH_MAX);
    name_max = pathconf(source, _PC_NAME_MAX);
    entry_size = sizeof(struct dirent) + name_max + 1;
    entry_ptr = malloc(entry_size);
    if (!entry_ptr) {
        perror("malloc");
        closedir(dir);
        return;
    }

    while (1) {
        int rc = readdir_r(dir, entry_ptr, &entry);
        if (rc != 0 || entry == NULL) break;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
                  
        char *src_path = malloc(path_max);
        if (!src_path) {
            perror("malloc");
            free(src_path);
            closedir(dir);
            return;
        }  
        char *dest_path = malloc(path_max);
        if (!dest_path) {
            perror("malloc");
            free(dest_path);
            closedir(dir);
            return;
        }    
        snprintf(src_path, path_max, "%s/%s", source, entry->d_name);
        snprintf(dest_path, path_max, "%s/%s", dest, entry->d_name);
        struct stat stat_buf;
        if (lstat(src_path, &stat_buf) == -1) {
            perror("lstat");
            continue;
        }
        if (S_ISDIR(stat_buf.st_mode)) {
            queue_push(TASK_PROCESS_DIR, src_path, dest_path);
        } else if (S_ISREG(stat_buf.st_mode)) {
            queue_push(TASK_COPY_FILE, src_path, dest_path);
        }
        free(src_path);  
        free(dest_path);      
    }

    free(entry_ptr);
    closedir(dir);
    pthread_cond_broadcast(&fileopen_cond);
}

void *worker(void *arg) {
    while (1) {
        Task *task = queue_pop();

        switch (task->type) {
            case TASK_COPY_FILE:
                if (task->source && task->dest)
                    copy_file(task->source, task->dest);
                break;
            case TASK_PROCESS_DIR:
                if (task->source && task->dest)
                    process_directory(task->source, task->dest);
                break;
            case TASK_EXIT:
                task_free(task);
                return NULL;
        }

        task_free(task);
        
        pthread_mutex_lock(&task_queue.mutex);
        task_queue.active_workers--;
        if (task_queue.pending_tasks == 0 && task_queue.active_workers == 0) {
            pthread_cond_broadcast(&task_queue.cond);
        }
        pthread_mutex_unlock(&task_queue.mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <dest>\n", argv[0]);
        return 1;
    }

    struct stat stat_buf;
    if (lstat(argv[1], &stat_buf) == -1 || !S_ISDIR(stat_buf.st_mode)) {
        fprintf(stderr, "Invalid source directory\n");
        return 1;
    }

    if (mkdir(argv[2], stat_buf.st_mode) == -1 && errno != EEXIST) {
        perror("mkdir");
        return 1;
    }
    queue_init();
    pthread_t threads[MAX_THREADS];

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    queue_push(TASK_PROCESS_DIR, argv[1], argv[2]);

    pthread_mutex_lock(&task_queue.mutex);
    while (task_queue.pending_tasks > 0 || task_queue.active_workers > 0) {
        pthread_cond_wait(&task_queue.cond, &task_queue.mutex);
    }
    pthread_mutex_unlock(&task_queue.mutex);
    for (int i = 0; i < MAX_THREADS; i++) {
        queue_push(TASK_EXIT, "", "");
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

