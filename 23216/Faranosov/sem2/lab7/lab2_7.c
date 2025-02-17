#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXSUBTHREADS 64


typedef struct paths {
	char* src;
	char* dst;
} paths;

typedef struct dir_data {
	DIR* dst;
	DIR* src;
	paths path;
} dir_data;

typedef struct file_data {
	int dst;
	int src;
} file_data;

void freeDirData(dir_data *d) {
	int res;
	res = closedir(d->dst);
	if (res != 0) {
		perror("closedir dst");
		pthread_exit(NULL);
	}

	res = closedir(d->src);
	if (res != 0) {
		perror("closedir src");
		pthread_exit(NULL);
	}

	free(d->path.dst);
	free(d->path.src);
	free(d);
}

void handler(char str[], int err_num, void *param) {
	char buf[256];
	strerror_r(err_num, buf, 256);
	fprintf(stderr, "%s error: %s\n", str, buf);
	pthread_exit(param);
}

void* copyDir(void* param);
void* copyFile(void* param);

void copyDirDir(pthread_t* thread, dir_data* data, pthread_attr_t* attr, char pathSrc[],
	char pathDst[], struct stat* statbuf);
void copyDirFile(pthread_t* thread, dir_data* data, pthread_attr_t* attr, char pathSrc[],
	char pathDst[], struct stat* statbuf);

void* copyFile(void* param) {
	file_data* data = param;
	char buf[1024];
	int res = 0;

	while (1) {
		res = read(data->src, buf, 1023);
		if (res == 0) {
			res = close(data->dst);
			if (res != 0) {
				perror("file dst close");
				pthread_exit(data);
			}
			res = close(data->src);
			if (res != 0) {
				perror("file src close");
				pthread_exit(data);
			}
			break;
		}
		else if (res < 0) {
			perror("read error");
			pthread_exit(data);
		}
		else {
			res = write(data->dst, buf, res);
			if (res < 0) {
				perror("write error");
				pthread_exit(data);
			}
		}
	}

	pthread_exit(data);
}

void check_malloc(void* param, void *retParam) {
	if (param == NULL) {
		fprintf(stderr, "thread malloc error");
		pthread_exit(retParam);
	}
}

void copyDirDir(pthread_t* thread, dir_data* data, pthread_attr_t *attr, char pathSrc[], 
									char pathDst[], struct stat* statbuf) {
	
	int res = 0;
	dir_data* new_dir = NULL;
	new_dir = malloc(sizeof(dir_data));
	check_malloc(new_dir, data);

	new_dir->src = opendir(pathSrc);
	if (new_dir->src == NULL) {
		perror("opendir source thread");
		pthread_exit(data);
	}

	new_dir->path.src = pathSrc;


	res = mkdir(pathDst, statbuf->st_mode);
	if (res != 0) {
		perror("mkdir error");
		pthread_exit(data);
	}

	new_dir->dst = opendir(pathDst);
	if (new_dir->dst == NULL) {
		perror("opendir dest error");
		pthread_exit(data);
	}

	new_dir->path.dst = pathDst;


	res = pthread_create(thread, attr, copyDir, new_dir);
	if (res != 0) handler("thread copyDir create", res, data);
}



void copyDirFile(pthread_t* thread, dir_data* data, pthread_attr_t* attr, char pathSrc[],
														char pathDst[], struct stat* statbuf) {
	int res = 0;
	file_data* new_file = NULL;
	new_file = malloc(sizeof(file_data));
	check_malloc(new_file, data);

	new_file->src = open(pathSrc, O_RDONLY);
	if (new_file->src == -1) {
		perror("open source thread");
		pthread_exit(data);
	}

	new_file->dst = open(pathDst, O_WRONLY | O_CREAT | O_TRUNC, statbuf->st_mode);
	if (new_file->dst == -1) {
		perror("open dest error");
		pthread_exit(data);
	}


	res = pthread_create(thread, attr, copyFile, new_file);
	if (res != 0) handler("pthread copyFile create", res, data);

}

void clear(void* param, char type) {
	dir_data* ddata = NULL;
	file_data* fdata = NULL;
	switch (type) {
	case 1:
		ddata = param;
		freeDirData(ddata);
		break;
	case 2:
		fdata = param;
		free(fdata);
		break;
	default:
		fprintf(stderr, "clear error switch");
		pthread_exit(NULL);
	}
}


void* copyDir(void* param) {
	dir_data* data = param;
	struct dirent* dp;
	struct stat statbuf;
	pthread_attr_t attr;
	int res, it = 0;
	char isNew = 0;
	void* retParam = NULL;


	pthread_t threads[MAXSUBTHREADS];
	char type[MAXSUBTHREADS];
	for (int i = 0; i < MAXSUBTHREADS; i++) type[i] = 0;

	res = pthread_attr_init(&attr);
	if (res != 0) handler("thread attr init", res, data);

	while ((dp = readdir(data->src)) != NULL) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

		if (isNew) {
			pthread_join(threads[it], &retParam);
			if (res != 0) handler("thread join", res, data);

			if (param != NULL) clear(param, type[it]);
		}

		char* buf = NULL;
		buf = malloc(sizeof(char) * 1024);
		check_malloc(buf, data);

		strcpy(buf, data->path.src);
		strcat(buf, "/");
		strcat(buf, dp->d_name);
		if (stat(buf, &statbuf) != 0) {
			printf("file = %s\n", buf);
			perror("stat thread error");
			pthread_exit(data);
		}

		if (S_ISDIR(statbuf.st_mode) || S_ISREG(statbuf.st_mode)) {

			char* buf2 = NULL;
			buf2 = malloc(sizeof(char) * 1024);
			check_malloc(buf2, data);

			strcpy(buf2, data->path.dst);
			strcat(buf2, "/");
			strcat(buf2, dp->d_name);

			if (S_ISDIR(statbuf.st_mode)) {
				copyDirDir(&threads[it], data, &attr, buf, buf2, &statbuf);
				type[it] = 1;
			}
			else {
				copyDirFile(&threads[it], data, &attr, buf, buf2, &statbuf);
				type[it] = 2;
				free(buf);
				free(buf2);
			}

			it++;
			it = it % MAXSUBTHREADS;
			if (it == 0) isNew = 1;
		}
		else {
			free(buf);
		}
	}

	if (isNew) it = MAXSUBTHREADS;
	for (int i = 0; i < it; i++) {
		res = pthread_join(threads[i], &retParam);
		if (res != 0) handler("thread join", res, data);

		if (param != NULL) clear(param, type[i]);
	}
	

	res = pthread_attr_destroy(&attr);
	if (res != 0) handler("thread attr destroy", res, data);

	

	pthread_exit(data);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("argc error");
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1], argv[2]) == 0) {
		printf("already");
		exit(EXIT_FAILURE);
	}
	
	struct stat st;
	if (stat(argv[1], &st) != 0) {
		perror("stat main error");
		exit(EXIT_FAILURE);
	}

	
	if (st.st_mode && S_IFDIR) {
		pthread_t thread;
		int res;
		dir_data* data = malloc(sizeof(dir_data));
		pthread_attr_t attr;

		if (data == NULL) {
			fprintf(stderr, "main malloc error\n");
			exit(EXIT_FAILURE);
		}
		
		data->path.dst = data->path.src = NULL;

		data->path.dst = malloc(sizeof(char) * 1024);
		data->path.src = malloc(sizeof(char) * 1024);
		if (data->path.dst == NULL || data->path.src == NULL) {
			fprintf(stderr, "main malloc error\n");
			exit(EXIT_FAILURE);
		}

		strcpy(data->path.src, argv[1]);
		strcpy(data->path.dst, argv[2]);

		data->dst = opendir(argv[2]);
		if (data->dst == NULL) {
			perror("opendir error for destination");
			exit(EXIT_FAILURE);
		}
		data->src = opendir(argv[1]);
		if (data->src == NULL) {
			perror("opendir error for source");
			exit(EXIT_FAILURE);
		}

		res = pthread_attr_init(&attr);
		if (res != 0) handler("main attr init", res, NULL);

		res = pthread_create(&thread, &attr, copyDir, data);
		if (res != 0) handler("main create", res, NULL);

		res = pthread_join(thread, (void*)&data);
		if (res != 0) handler("main join", res, NULL);

		res = closedir(data->dst);
		if (res != 0) {
			perror("closedir dst");
			pthread_exit(NULL);
		}

		res = closedir(data->src);
		if (res != 0) {
			perror("closedir src");
			pthread_exit(NULL);
		}

		free(data->path.dst);
		free(data->path.src);
		free(data);

		res = pthread_attr_destroy(&attr);
		if (res != 0) handler("main attr destroy", res, NULL);
	}
	else {
		printf("wrong type");
		exit(EXIT_FAILURE);
	}

	pthread_exit(NULL);
}