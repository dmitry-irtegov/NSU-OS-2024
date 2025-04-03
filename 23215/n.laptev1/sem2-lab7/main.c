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

#define BUFSIZE 4096

char *srcdir, *dstdir;

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

int create_thread_with_retry(pthread_t *threadp, void *(*body)(void *), void *arg) {
    int err;
    do {
        err = pthread_create(threadp, NULL, body, arg);
        if (err != EAGAIN) return err;
        printf("Waiting for free threads\n");
        sleep(1);
    } while (err != 0);
    pthread_detach(*threadp);
    return 0;
}

void *copy_file(void *param) {
    char *filename = (char *)param;
    char srcpathname[strlen(srcdir) + strlen(filename) + 2];
    char dstpathname[strlen(dstdir) + strlen(filename) + 2];
    int srcfile, dstfile;
    char buf[BUFSIZE];
    struct stat stat_buf;
    ssize_t was_read;

    snprintf(srcpathname, sizeof(srcpathname), "%s/%s", srcdir, filename);
    snprintf(dstpathname, sizeof(dstpathname), "%s/%s", dstdir, filename);
    free(param);

    pthread_mutex_lock(&fileopen_mutex);
    do {
        srcfile = open(srcpathname, O_RDONLY);
        if (srcfile < 0 && errno != EMFILE) {
            perror(srcpathname);
            free(filename);
            pthread_mutex_unlock(&fileopen_mutex);
            return NULL;
        }
        if (srcfile >= 0) {
            fstat(srcfile, &stat_buf);
            dstfile = open(dstpathname, O_WRONLY | O_CREAT, stat_buf.st_mode);// Тип файла и права доступа (File type and mode).
            if (dstfile < 0 && errno != EMFILE) {
                perror(dstpathname);
                close(srcfile);
                pthread_mutex_unlock(&fileopen_mutex);
                pthread_cond_signal(&fileopen_cond);
                return NULL;
            }
        }
        if (srcfile < 0 || dstfile < 0) {
            if (srcfile >= 0) {
                close(srcfile);
            }    
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        }
    } while (srcfile < 0 || dstfile < 0);
    pthread_mutex_unlock(&fileopen_mutex);

    while ((was_read = read(srcfile, buf, sizeof buf)) > 0) {
        if (write(dstfile, buf, was_read) < 0) break;
    }
    close(srcfile);
    close(dstfile);
    pthread_cond_signal(&fileopen_cond);
    return NULL;
}

void *copy_directory(void *param) {
    char *dirname = (char *)param;
    char srcpathname[strlen(srcdir) + strlen(dirname) + 2];
    char dstpathname[strlen(dstdir) + strlen(dirname) + 2];
    DIR *srcdirp;
    struct stat stat_buf;

    snprintf(srcpathname, sizeof(srcpathname), "%s/%s", srcdir, dirname);
    snprintf(dstpathname, sizeof(dstpathname), "%s/%s", dstdir, dirname);
    free(param);

    pthread_mutex_lock(&fileopen_mutex);
    do {
        srcdirp = opendir(srcpathname);
        if (srcdirp == NULL && errno != EMFILE) {
            perror(srcpathname);
            pthread_mutex_unlock(&fileopen_mutex);
            return NULL;
        }
        if (srcdirp == NULL && errno == EMFILE) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        }
    } while (srcdirp == NULL);
    pthread_mutex_unlock(&fileopen_mutex);

    mkdir(dstpathname, 0755);

    size_t max_fname_length = pathconf(srcpathname, _PC_NAME_MAX);
    if (max_fname_length == -1) {
        perror("pathconf");
        return NULL;
    }
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
    pthread_cond_signal(&fileopen_cond);
    return NULL;
}

int main(int argc, char **argv) {
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