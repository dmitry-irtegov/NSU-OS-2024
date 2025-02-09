#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

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

void handler(char str[], int err_num) {
	char buf[256];
	strerror_r(err_num, buf, 256);
	fprintf(stderr, "%s error: %s\n", str, buf);
	pthread_exit(NULL);
}

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
				pthread_exit(NULL);
			}
			res = close(data->src);
			if (res != 0) {
				perror("file src close");
				pthread_exit(NULL);
			}
			break;
		}
		else if (res < 0) {
			perror("read error");
			pthread_exit(NULL);
		}
		else {
			res = write(data->dst, buf, res);
			if (res < 0) {
				perror("write error");
				pthread_exit(NULL);
			}
		}
	}

	pthread_exit(NULL);
}

void check_malloc(void* param) {
	if (param == NULL) {
		fprintf(stderr, "thread malloc error");
		pthread_exit(NULL);
	}
}

void* copyDir(void* param) {
	dir_data* data = param;
	struct dirent* dp;
	file_data* new_file;
	dir_data* new_dir;
	struct stat statbuf;
	pthread_attr_t attr;
	int res;

	res = pthread_attr_init(&attr);
	if (res != 0) handler("thread attr init", res);

	while ((dp = readdir(data->src)) != NULL) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

		char* buf = NULL;
		buf = malloc(sizeof(char) * 1024);
		check_malloc(buf);

		strcpy(buf, data->path.src);
		strcat(buf, "/");
		strcat(buf, dp->d_name);
		if (stat(buf, &statbuf) != 0) {
			printf("file = %s\n", buf);
			perror("stat thread error");
			pthread_exit(NULL);
		}

		if (S_ISDIR(statbuf.st_mode) || S_ISREG(statbuf.st_mode)) {
			if (S_ISDIR(statbuf.st_mode)) {
				new_dir = NULL;
				new_dir = malloc(sizeof(dir_data));
				check_malloc(new_dir);

				new_dir->src = opendir(buf);
				if (new_dir->src == NULL) {
					perror("opendir source thread");
					pthread_exit(NULL);
				}

				new_dir->path.src = buf;

			}
			else {
				new_file = NULL;
				new_file = malloc(sizeof(file_data));
				check_malloc(new_file);

				new_file->src = open(buf, O_RDONLY);
				if (new_file->src == -1) {
					perror("open source thread");
					pthread_exit(NULL);
				}

				free(buf);
			}

			buf = NULL;
			buf = malloc(sizeof(char) * 1024);
			check_malloc(buf);

			strcpy(buf, data->path.dst);
			strcat(buf, "/");
			strcat(buf, dp->d_name);

			if (S_ISDIR(statbuf.st_mode)) {
				res = mkdir(buf, statbuf.st_mode);
				if (res != 0) {
					perror("mkdir error");
					pthread_exit(NULL);
				}

				new_dir->dst = opendir(buf);
				if (new_dir->dst == NULL) {
					perror("opendir dest error");
					pthread_exit(NULL);
				}

				new_dir->path.dst = buf;

				pthread_t thread;

				res = pthread_create(&thread, &attr, copyDir, new_dir);
				if (res != 0) handler("thread copyDir create", res);
			}
			else {
				new_file->dst = open(buf, O_WRONLY | O_CREAT | O_TRUNC, statbuf.st_mode);
				if (new_file->dst == -1) {
					perror("open dest error");
					pthread_exit(NULL);
				}

				pthread_t thread;

				res = pthread_create(&thread, &attr, copyFile, new_file);
				if (res != 0) handler("pthread copyFile create", res);

				free(buf);
			}
		}
		else {
			free(buf);
		}
	}

	res = pthread_attr_destroy(&attr);
	if (res != 0) handler("thread attr destroy", res);

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
	pthread_exit(NULL);
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
		if (res != 0) handler("main attr init", res);

		res = pthread_create(&thread, &attr, copyDir, data);
		if (res != 0) handler("main create thread", res);

		res = pthread_attr_destroy(&attr);
		if (res != 0) handler("main attr destroy", res);
	}
	else {
		printf("wrong type");
		exit(EXIT_FAILURE);
	}

	pthread_exit(NULL);
}