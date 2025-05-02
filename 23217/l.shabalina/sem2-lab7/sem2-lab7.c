#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>

#define NAME_MAXX 256
#define BUFFER_SIZE 65536
#define MAX_THREADS 8
#define QUEUE_SIZE 1024
#define handle_error(en, msg) { errno = en; perror(msg); exit(EXIT_FAILURE); }

typedef struct {
    char src_path[PATH_MAX];
    char dst_path[PATH_MAX];
    int is_directory; 
} copy_task_t;

typedef struct {
    copy_task_t *tasks;
    int front;
    int rear;
    int count;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} task_queue_t;

task_queue_t queue;
pthread_t threads[MAX_THREADS];
int running = 1;

void queue_init(task_queue_t *q, int size) {
    q->tasks = malloc(size * sizeof(copy_task_t));
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    q->size = size;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

void enqueue(task_queue_t *q, copy_task_t task) {
    pthread_mutex_lock(&q->lock);
    while (q->count == q->size) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    q->tasks[q->rear] = task;
    q->rear = (q->rear + 1) % q->size;
    q->count++;
    
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

copy_task_t dequeue(task_queue_t *q) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0 && running) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    
    if (!running) {
        pthread_mutex_unlock(&q->lock);
        copy_task_t empty_task = {0};
        return empty_task;
    }
    
    copy_task_t task = q->tasks[q->front];
    q->front = (q->front + 1) % q->size;
    q->count--;
    
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    return task;
}

void copy_file(copy_task_t *task) {
    char buffer[BUFFER_SIZE];
    struct stat stat_buf;
    int src_fd = -1, dst_fd = -1;

    if (stat(task->src_path, &stat_buf) != 0) {
        perror("stat failed");
        return;
    }

    while ((src_fd = open(task->src_path, O_RDONLY)) == -1) {
        if (errno != EMFILE) {
            perror("open source failed");
            return;
        }
        sleep(1);
    }

    while ((dst_fd = open(task->dst_path, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode)) == -1) {
        if (errno != EMFILE) {
            perror("open destination failed");
            close(src_fd);
            return;
        }
        sleep(1);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        char *ptr = buffer;
        ssize_t bytes_remaining = bytes_read;
        
        while (bytes_remaining > 0) {
            ssize_t bytes_written = write(dst_fd, ptr, bytes_remaining);
            if (bytes_written <= 0) {
                perror("write failed");
                close(src_fd);
                close(dst_fd);
                return;
            }
            ptr += bytes_written;
            bytes_remaining -= bytes_written;
        }
    }

    if (bytes_read == -1) {
        perror("read failed");
    }

    close(src_fd);
    close(dst_fd);
}

void process_directory(copy_task_t *task) {
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    size_t entry_size;

    if (mkdir(task->dst_path, 0755) != 0 && errno != EEXIST) {
        perror("mkdir failed");
        return;
    }

    while ((dir = opendir(task->src_path)) == NULL) {
        if (errno != EMFILE) {
            perror("opendir failed");
            return;
        }
        sleep(1);
    }

    entry_size = offsetof(struct dirent, d_name) + NAME_MAXX + 1;
    entry = malloc(entry_size);
    if (!entry) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    struct dirent *result;
    int readdir_result;
    while ((readdir_result = readdir_r(dir, entry, &result)) == 0 && result != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_child_path[PATH_MAX];
        char dst_child_path[PATH_MAX];
        struct stat stat_buf;

        if (snprintf(src_child_path, PATH_MAX, "%s/%s", task->src_path, entry->d_name) >= PATH_MAX ||
            snprintf(dst_child_path, PATH_MAX, "%s/%s", task->dst_path, entry->d_name) >= PATH_MAX) {
            fprintf(stderr, "Path too long: %s/%s\n", task->src_path, entry->d_name);
            continue;
        }

        if (stat(src_child_path, &stat_buf) != 0) {
            continue;
        }

        copy_task_t new_task;
        strncpy(new_task.src_path, src_child_path, PATH_MAX);
        strncpy(new_task.dst_path, dst_child_path, PATH_MAX);

        if (S_ISDIR(stat_buf.st_mode)) {
            new_task.is_directory = 1;
        } else if (S_ISREG(stat_buf.st_mode)) {
            new_task.is_directory = 0;
        } else {
            continue;
        }
        
        enqueue(&queue, new_task);
    }

    if (readdir_result != 0) {
        perror("readdir_r failed");
    }

    free(entry);
    closedir(dir);
}

void* worker_thread(void *arg) {
    while (running) {
        copy_task_t task = dequeue(&queue);
        if (!running && queue.count == 0) { 
            break;
        }

        if (task.is_directory) {
            process_directory(&task);
        } else {
            copy_file(&task);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "use: %s source destination\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct stat stat_buf;
    if (stat(argv[1], &stat_buf) != 0) {
        handle_error(errno, "stat failed");
    }

    if (!S_ISDIR(stat_buf.st_mode)) {
        fprintf(stderr, "source is not a directory\n");
        exit(EXIT_FAILURE);
    }

    queue_init(&queue, QUEUE_SIZE);

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, NULL) != 0) {
            handle_error(errno, "pthread_create failed");
        }
    }

    copy_task_t root_task;
    if (realpath(argv[1], root_task.src_path) == NULL) {
        handle_error(errno, "realpath source failed");
    }
    if (strlen(argv[2]) >= PATH_MAX) {
        fprintf(stderr, "destination path too long\n");
        exit(EXIT_FAILURE);
    }
    strncpy(root_task.dst_path, argv[2], PATH_MAX);
    root_task.is_directory = 1;
    
    enqueue(&queue, root_task);

    while (queue.count > 0) {
        sleep(1);
    }

    running = 0;
    pthread_cond_broadcast(&queue.not_empty);
    
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    free(queue.tasks);
    pthread_mutex_destroy(&queue.lock);
    pthread_cond_destroy(&queue.not_empty);
    pthread_cond_destroy(&queue.not_full);

    return 0;
}

