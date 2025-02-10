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

typedef enum { TASK_COPY_FILE, TASK_PROCESS_DIR } TaskType;

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
    int shutdown;
    int active_workers;
} TaskQueue;

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

TaskQueue task_queue;

void queue_init() {
    task_queue.head = task_queue.tail = NULL;
    pthread_mutex_init(&task_queue.mutex, NULL);
    pthread_cond_init(&task_queue.cond, NULL);
    task_queue.shutdown = 0;
    task_queue.active_workers = 0;
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
    pthread_cond_signal(&task_queue.cond);
    pthread_mutex_unlock(&task_queue.mutex);
}

Task *queue_pop() {
    pthread_mutex_lock(&task_queue.mutex);
    while (!task_queue.head && !task_queue.shutdown) {
        pthread_cond_wait(&task_queue.cond, &task_queue.mutex);
    }
    
    Task *task = NULL;
    if (task_queue.head) {
        task = task_queue.head;
        task_queue.head = task->next;
        if (!task_queue.head) task_queue.tail = NULL;
        task_queue.active_workers++;
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

int safe_open(const char *path, int flags, mode_t mode) {
    int fd;
    int retries = MAX_RETRIES;
    
    while (1) {
        pthread_mutex_lock(&fileopen_mutex);
        fd = open(path, flags, mode);
        if (fd == -1 && errno == EMFILE && retries-- > 0) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
            pthread_mutex_unlock(&fileopen_mutex);
            sleep(1);
            continue;
        }
        pthread_mutex_unlock(&fileopen_mutex);
        break;
    }
    return fd;
}

void copy_file(const char *source, const char *dest) {
    int src_fd = safe_open(source, O_RDONLY, 0);
    if (src_fd == -1) {
        perror("open source");
        return;
    }

    struct stat stat_buf;
    if (fstat(src_fd, &stat_buf) == -1) {
        perror("fstat");
        close(src_fd);
        return;
    }

    int dest_fd = safe_open(dest, O_WRONLY|O_CREAT|O_TRUNC, stat_buf.st_mode & 0777);
    if (dest_fd == -1) {
        perror("open dest");
        close(src_fd);
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
    size_t entry_size;
    struct dirent *entry_ptr;

    mkdir(dest, 0755);
    
    pthread_mutex_lock(&fileopen_mutex);
    dir = opendir(source);
    pthread_mutex_unlock(&fileopen_mutex);
    
    if (!dir) {
        if (errno == EMFILE) {
            sleep(1);
            process_directory(source, dest);
        } else {
            perror("opendir");
        }
        return;
    }

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

        char src_path[PATH_MAX], dest_path[PATH_MAX];
        snprintf(src_path, PATH_MAX, "%s/%s", source, entry->d_name);
        snprintf(dest_path, PATH_MAX, "%s/%s", dest, entry->d_name);

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
    }

    free(entry_ptr);
    closedir(dir);
    pthread_cond_signal(&fileopen_cond);
}

void *worker(void *arg) {
    while (1) {
        Task *task = queue_pop();
        if (!task) break;

        switch (task->type) {
            case TASK_COPY_FILE:
                copy_file(task->source, task->dest);
                break;
            case TASK_PROCESS_DIR:
                process_directory(task->source, task->dest);
                break;
        }

        task_free(task);
        pthread_mutex_lock(&task_queue.mutex);
        task_queue.active_workers--;
        pthread_cond_signal(&task_queue.cond);
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

    if (mkdir(argv[2], stat_buf.st_mode & 0777) == -1 && errno != EEXIST) {
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
    while (!task_queue.shutdown) {
        if (task_queue.active_workers == 0 && !task_queue.head) {
            task_queue.shutdown = 1;
            pthread_cond_broadcast(&task_queue.cond);
            break;
        }
        pthread_cond_wait(&task_queue.cond, &task_queue.mutex);
    }
    pthread_mutex_unlock(&task_queue.mutex);

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&task_queue.mutex);
    pthread_cond_destroy(&task_queue.cond);
    pthread_mutex_destroy(&fileopen_mutex);
    pthread_cond_destroy(&fileopen_cond);
    return 0;
}
