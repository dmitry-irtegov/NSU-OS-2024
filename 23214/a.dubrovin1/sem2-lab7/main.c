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
#define SLEEPTIME 1
 
char *srcdir, *dstdir; 
pthread_mutex_t fileopen_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileopen_cond = PTHREAD_COND_INITIALIZER;

int create_thread_with_retry(pthread_t *threadp, pthread_attr_t *attrp, void *(*body)(void *), void *arg) {
    int code = -1;
    while (code != 0) {
        code = pthread_create(threadp, attrp, body, arg);
        if (code == EAGAIN) {
            fprintf(stdout, "Not enough recources to create thread, retrying\n");
            sleep(SLEEPTIME);
        }

        else if (code != 0) {
            char buf[256];
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "Unable to create a thread: %s\n", buf);
            return code;
        }
    }
    return 0;
}

void *copy_file(void * param) {
    char * filename = (char *)param;
    char *srcpathname = malloc(strlen(srcdir)+strlen(filename)+2);
    if (srcpathname == NULL) {
        fprintf(stderr, "Malloc error\n");
        return (void *) 1;
    }
    char *dstpathname = malloc(strlen(dstdir)+strlen(filename)+2);
    if (dstpathname == NULL) {
        fprintf(stderr, "Malloc error\n");
        free(srcpathname);
        return (void *) 1;
    }

    int srcfile = -1;
    int dstfile = -1;
    char buf[BUFSIZE];
    struct stat stat;
     
    strcat(strcat(strcpy(srcpathname, srcdir), "/"), filename);
    strcat(strcat(strcpy(dstpathname, dstdir), "/"), filename);

    pthread_mutex_lock(&fileopen_mutex);
    while (dstfile<0 || srcfile<0) {
        srcfile = open(srcpathname, O_RDONLY, 0);
        if (srcfile < 0 && errno!=EMFILE) {
            perror(dstpathname);
            pthread_mutex_unlock(&fileopen_mutex);
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
                return NULL;
            }
        }

        if (srcfile < 0 || dstfile < 0) {
            if (srcfile>0) {
                close(srcfile);
                pthread_cond_signal(&fileopen_cond);
            }
            pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
        }
    }
    
    pthread_mutex_unlock(&fileopen_mutex);

    int bytes_read;
    while(1) {
        bytes_read = read(srcfile, buf, sizeof buf);
        if (bytes_read == 0) {
            break;
        } else if (bytes_read < 0) {
            perror("Read error");
        }

        if (write(dstfile, buf, bytes_read)<0) {
            perror("Write error");
            break;
        }
    }

    close(srcfile);
    close(dstfile);
    pthread_cond_signal(&fileopen_cond);
    return NULL;
}

void * copy_directory(void * param) {
    char *srcpathname = malloc(strlen(srcdir)+strlen(param)+2);
    char *dstpathname = malloc(strlen(dstdir)+strlen(param)+2);
    char *dirname = malloc(strlen(param)+1);
    DIR * srcdirptr = NULL;
    struct stat stat;
    size_t max_fname_length;
   
    strcpy(dirname, param); 
    strcat(strcat(strcpy(srcpathname, srcdir), "/"), dirname);
    strcat(strcat(strcpy(dstpathname, dstdir), "/"), dirname);
    free(param);
        
    pthread_mutex_lock(&fileopen_mutex);
    while(srcdirptr == NULL) {
        srcdirptr = opendir(srcpathname);
        if (srcdirptr == NULL) {
            if (errno != EMFILE){
                pthread_mutex_unlock(&fileopen_mutex);
                perror("Directory open error");
                return NULL;
            } else {
                pthread_cond_wait(&fileopen_cond, &fileopen_mutex);
            }
        }
    }

    pthread_mutex_unlock(&fileopen_mutex);
        
    if (mkdir(dstpathname, 0700) != 0) {
        perror("Mkdir error");
        closedir(srcdirptr);
        pthread_cond_signal(&fileopen_cond);
        return NULL;
    }
        
    max_fname_length = pathconf(srcpathname, _PC_NAME_MAX);
    
    while (1) {
        struct dirent * entry = malloc(sizeof(struct dirent)+max_fname_length+2);       
        struct dirent * result;
        result = readdir_r(srcdirptr, entry, NULL); 

        if (result == NULL){
            break;
        }
        if (result != NULL) {
            char * relpathname;
            char * abspathname = malloc(strlen(srcpathname)+strlen(result->d_name)+2);
            pthread_t thread;
                
            if (strcmp(result->d_name, ".") == 0 || strcmp(result->d_name, "..") == 0) {
                continue;
            }
            
            strcat(strcat(strcpy(abspathname, srcpathname), "/"), result->d_name);
                
            lstat(abspathname, &stat);
                
            relpathname = strdup(abspathname+strlen(srcdir)+1);
            int code;
            if(S_ISREG(stat.st_mode)) {         
                code = create_thread_with_retry(&thread, NULL, &copy_file, relpathname);
                if (code !=0 ) {
                    fprintf(stderr, "creating thread: %s\n", strerror(code));
                    free(relpathname);
                    return NULL;
                }
            } else if(S_ISDIR(stat.st_mode)) {
                code = create_thread_with_retry(&thread, NULL, &copy_directory, relpathname);
                if (code != 0) {
                    fprintf(stderr, "creating thread: %s\n", strerror(code));
                    free(relpathname);
                    return NULL;
                }
            } else {
                free(relpathname);
            }
        }
    }

    closedir(srcdirptr);
    pthread_cond_signal(&fileopen_cond);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "There should be 2 arguments");
        exit(1);
    }

    srcdir = argv[1];
    dstdir = argv[2];
    if (mkdir(dstdir, 0700) != 0) {
        perror(dstdir);
        exit(0);
    }

    copy_directory((void*) strdup(""));
    pthread_exit(NULL);
}