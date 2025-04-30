#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#define BUFSIZE 4096
#define FILEDESCLIMIT 5

char *srcdir, *dstdir;

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

int create_thread_with_retry(pthread_t *threadp, void *(*body)(void *), void *arg) {
    int err;
    do {
        err = pthread_create(threadp, NULL, body, arg);
        if (err == EAGAIN) {
            printf("[create_thread] Resource busy, retrying...\n");
            sleep(1);
        } else if (err != 0) {
            fprintf(stderr, "[create_thread] pthread_create error: %s\n", strerror(err));
            return err;
        }
    } while (err != 0);
    pthread_detach(*threadp);
    return 0;
}

void *copy_file(void *param) {
    char *filename = (char *)param;
    char srcpathname[strlen(srcdir) + strlen(filename) + 2];
    char dstpathname[strlen(dstdir) + strlen(filename) + 2];
    int srcfile = -1, dstfile = -1;
    char buf[BUFSIZE];
    struct stat stat_buf;
    ssize_t was_read;

    snprintf(srcpathname, sizeof(srcpathname), "%s/%s", srcdir, filename);
    snprintf(dstpathname, sizeof(dstpathname), "%s/%s", dstdir, filename);
    free(param);

    printf("[copy_file] Trying to open src '%s' and dst '%s'\n", srcpathname, dstpathname);
    pthread_mutex_lock(&fileopen_mutex);
    do {
        srcfile = open(srcpathname, O_RDONLY);
        if (srcfile < 0) {
            if (errno == EMFILE) {
                printf("[copy_file] EMFILE when opening src '%s', waiting...\n", srcpathname);
            } else {
                perror("[copy_file] open src error");
                pthread_mutex_unlock(&fileopen_mutex);
                free(filename);
                return NULL;
            }
        } else {
            printf("[copy_file] Opened src '%s' (fd=%d)\n", srcpathname, srcfile);
            if (fstat(srcfile, &stat_buf) < 0) {
                perror("[copy_file] fstat error");
            }
            dstfile = open(dstpathname, O_WRONLY | O_CREAT, stat_buf.st_mode);
            if (dstfile < 0) {
                if (errno == EMFILE) {
                    printf("[copy_file] EMFILE when opening dst '%s'...\n", dstpathname);
                    close(srcfile);
                    srcfile = -1;
                } else {
                    perror("[copy_file] open dst error");
                    close(srcfile);
                    pthread_mutex_unlock(&fileopen_mutex);
                    pthread_cond_signal(&fileopen_cond);
                    return NULL;
                }
            } else {
                printf("[copy_file] Opened dst '%s' (fd=%d)\n", dstpathname, dstfile);
            }
        }
        if (srcfile < 0 || dstfile < 0) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        }
    } while (srcfile < 0 || dstfile < 0);
    pthread_mutex_unlock(&fileopen_mutex);
    printf("[copy_file] Starting copy '%s' -> '%s'\n", srcpathname, dstpathname);

    while ((was_read = read(srcfile, buf, BUFSIZE)) > 0) {
        if (write(dstfile, buf, was_read) < 0) {
            perror("[copy_file] write error");
            break;
        }
    }

    close(srcfile);
    close(dstfile);
    printf("[copy_file] Finished copy '%s'\n", srcpathname, dstpathname);
    pthread_cond_signal(&fileopen_cond);
    return NULL;
}

void *copy_directory(void *param) {
    char *dirname = (char *)param;
    char srcpathname[strlen(srcdir) + strlen(dirname) + 2];
    char dstpathname[strlen(dstdir) + strlen(dirname) + 2];
    DIR *srcdirp = NULL;
    struct stat stat_buf;

    snprintf(srcpathname, sizeof(srcpathname), "%s/%s", srcdir, dirname);
    snprintf(dstpathname, sizeof(dstpathname), "%s/%s", dstdir, dirname);
    free(param);

    printf("[copy_dir] Trying to open dir '%s'\n", srcpathname);
    pthread_mutex_lock(&fileopen_mutex);
    do {
        srcdirp = opendir(srcpathname);
        if (srcdirp == NULL) {
            if (errno == EMFILE) {
                printf("[copy_dir] EMFILE when opening dir '%s', waiting...\n", srcpathname);
                pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
            } else {
                perror("[copy_dir] opendir error");
                pthread_mutex_unlock(&fileopen_mutex);
                return NULL;
            }
        } else {
            printf("[copy_dir] Opened dir '%s'\n", srcpathname);
        }
    } while (srcdirp == NULL);
    pthread_mutex_unlock(&fileopen_mutex);

    if (mkdir(dstpathname, 0755) == 0) {
        printf("[copy_dir] Created directory '%s'\n", dstpathname);
    }

    size_t max_fname_length = pathconf(srcpathname, _PC_NAME_MAX);
    struct dirent *entry = malloc(sizeof(struct dirent) + max_fname_length + 2);
    struct dirent *result;

    while (readdir_r(srcdirp, entry, &result) == 0 && result != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char absolutepathname[strlen(srcpathname) + strlen(entry->d_name) + 2];
        snprintf(absolutepathname, sizeof(absolutepathname), "%s/%s", srcpathname, entry->d_name);
        lstat(absolutepathname, &stat_buf);
        char *relativepathname = strdup(absolutepathname + strlen(srcdir) + 1);
        pthread_t thread;
        if (S_ISREG(stat_buf.st_mode)) {
            create_thread_with_retry(&thread, &copy_file, relativepathname);
        } else if (S_ISDIR(stat_buf.st_mode)) {
            create_thread_with_retry(&thread, &copy_directory, relativepathname);
        } else {
            free(relativepathname);
        }
    }

    free(entry);
    closedir(srcdirp);
    printf("[copy_dir] Closed dir '%s'\n", srcpathname);
    pthread_cond_signal(&fileopen_cond);
    return NULL;
}


int main(int argc, char **argv) {
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = FILEDESCLIMIT;  // Ограничим до 5 дескрипторов
    setrlimit(RLIMIT_NOFILE, &rl);
    if (argc < 3) {
        fprintf(stderr, "Usage: %s srcdir dstdir\n", argv[0]);
        exit(0);
    }
    srcdir = argv[1];
    dstdir = argv[2];
    mkdir(dstdir, 0755);
    copy_directory(strdup(""));
    pthread_exit(NULL);
}