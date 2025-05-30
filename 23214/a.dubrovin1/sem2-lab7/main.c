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

char* srcdir;
char* dstdir;

pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

int create_thread_with_retry(pthread_t *threadp, pthread_attr_t *attrp, void *(*body)(void *), void *arg) {
    int code = -1;
    while (code != 0) {
        code = pthread_create(threadp, attrp, body, arg);
        if (code != EAGAIN) {
            return code;
        } else {
            printf("Failed to create a thread, retrying\n");
            sleep(1);
        }
    }
    return 0;
}

void *copy_file(void * param) {
    char * filename = (char*) param;
    char *srcpathname = malloc(strlen(srcdir)+strlen(filename)+2);
    char *dstpathname = malloc(strlen(dstdir)+strlen(filename)+2);

    if (!srcpathname || !dstpathname) {
        perror("malloc");
        free(srcpathname);
        free(dstpathname);
        free(param);
        return NULL;
    }

    int srcfile = -1;
    int dstfile = -1;
    char buf[BUFSIZE];
    struct stat stat;
    ssize_t bytes_read;
    
    strcat(strcat(strcpy(srcpathname, srcdir), "/"), filename);
    strcat(strcat(strcpy(dstpathname, dstdir), "/"), filename);

    free(param);

    pthread_mutex_lock(&fileopen_mutex);
    while(dstfile < 0 || srcfile < 0) {
        srcfile = open(srcpathname, O_RDONLY, 0);
        if (srcfile < 0 && errno != EMFILE) {
            perror(dstpathname);
            pthread_mutex_unlock(&fileopen_mutex);
            free(srcpathname);
            free(dstpathname);
            return NULL;
        } 

        if (srcfile > 0) {
            fstat(srcfile, &stat);
            dstfile = open(dstpathname, O_WRONLY | O_CREAT, stat.st_mode);
            if (dstfile < 0 && errno != EMFILE) {
                perror(dstpathname);
                close(srcfile);
                pthread_mutex_unlock(&fileopen_mutex);
                pthread_cond_signal(&fileopen_cond);
                free(srcpathname);
                free(dstpathname);
                return NULL;
            }
        }

        if (srcfile < 0 || dstfile < 0) {
            if (srcfile > 0) {
                close(srcfile);
                pthread_cond_signal(&fileopen_cond);
            }
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        }
    }
    pthread_mutex_unlock(&fileopen_mutex);
        
    while((bytes_read = read(srcfile, buf, sizeof buf)) > 0) {
        if (write(dstfile, buf, bytes_read) < 0) {
            perror(dstpathname);
            break;
        }
    }
    
    if (bytes_read < 0) {
        perror(srcpathname);
    }
    
    close(srcfile);
    close(dstfile);
    pthread_cond_signal(&fileopen_cond);

    free(srcpathname);
    free(dstpathname);
    return NULL;
}

void * copy_directory(void * param) {
    char* srcpathname = malloc(strlen(srcdir)+strlen(param)+2);
    char* dstpathname = malloc(strlen(dstdir)+strlen(param)+2);
    char* dirname = malloc(strlen(param)+1);

    if (!srcpathname || !dstpathname || !dirname) {
        perror("malloc");
        free(srcpathname);
        free(dstpathname);
        free(dirname);
        free(param);
        return NULL;
    }

    DIR * srcdirp = NULL;
    struct stat stat;

    strcpy(dirname, param);
    strcat(strcat(strcpy(srcpathname, srcdir), "/"), dirname);
    strcat(strcat(strcpy(dstpathname, dstdir), "/"), dirname);
    free(param);
    
    pthread_mutex_lock(&fileopen_mutex);
    while(srcdirp == NULL) {
        srcdirp = opendir(srcpathname);
        if (srcdirp == NULL && errno != EMFILE) {
            pthread_mutex_unlock(&fileopen_mutex);
            perror(srcpathname);
            free(srcpathname);
            free(dstpathname);
            free(dirname);
            return NULL;
        } else if (srcdirp == NULL && errno == EMFILE) {
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        }
    }
    pthread_mutex_unlock(&fileopen_mutex);
    
    if (mkdir(dstpathname, 0700) < 0) {
        if (errno == EEXIST) {
            if (access(dstdir, W_OK | X_OK)) {
                fprintf(stderr, "Failed to open directory %s: access denied\n", dstdir);
                closedir(srcdirp);
                free(srcpathname);
                free(dstpathname);
                free(dirname);
                return NULL;
            }
        } else {
            perror(dstpathname);
            closedir(srcdirp);
            pthread_cond_signal(&fileopen_cond);
            free(srcpathname);
            free(dstpathname);
            free(dirname);
            return NULL;
        }
    }

    struct dirent *entry;
    while((entry = readdir(srcdirp)) != NULL) {
        char* relativepathname;
        char* absolutepathname = malloc(strlen(srcpathname)+strlen(entry->d_name)+2);
        pthread_t thread;
        
        if (!absolutepathname) {
            perror("malloc");
            continue;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            free(absolutepathname);
            continue;
        }

        strcat(strcat(strcpy(absolutepathname, srcpathname), "/"), entry->d_name);
        
        if (lstat(absolutepathname, &stat) < 0) {
            perror(absolutepathname);
            free(absolutepathname);
            continue;
        }
        
        relativepathname = strdup(absolutepathname+strlen(srcdir)+1);
        free(absolutepathname);

        if (!relativepathname) {
            perror("strdup");
            continue;
        }

        int code;
        if(S_ISREG(stat.st_mode)) {         
            code = create_thread_with_retry(&thread, NULL, &copy_file, relativepathname);
        } else if(S_ISDIR(stat.st_mode)) {
            code = create_thread_with_retry(&thread, NULL, &copy_directory, relativepathname);
        } else {
            free(relativepathname);
            continue;
        }

        if (code != 0) {
            fprintf(stderr, "creating thread: %s\n", strerror(code));
            free(relativepathname);
            closedir(srcdirp);
            free(srcpathname);
            free(dstpathname);
            free(dirname);
            return NULL;
        }
    }

    closedir(srcdirp);
    pthread_cond_signal(&fileopen_cond);

    free(srcpathname);
    free(dstpathname);
    free(dirname);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Input has %d arguments instead of 3\n", argc);
        exit(0);
    }

    srcdir = argv[1];
    dstdir = argv[2];
    if(mkdir(dstdir, 0700) < 0) {
        if (errno == EEXIST) {
            if (access(dstdir, W_OK | X_OK)) {
                fprintf(stderr, "Failed to open directory %s: access denied\n", dstdir);
                exit(0);
            }
        } else {
            perror(dstdir);
            exit(0);
        }
    }

    char* temp = malloc(sizeof(char));
    if (!temp) {
        perror("malloc");
        exit(1);
    }

    temp[0] = '\0';
    copy_directory(temp);
    pthread_exit(NULL);
}
