#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stddef.h"
#include "unistd.h"
#include <ctype.h>
#include <poll.h>

#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/un.h>
#include "sys/syscall.h"
#include <sys/wait.h>
#include <errno.h>


typedef struct job {
	pid_t id;
	char isUsing;
} job;

struct pollfd pfds[20];


job ids[20];


int findJob(pid_t id) {
	for (int i = 0; i < 20; i++) {
		if (ids[i].id == id) {
			return i;
		}
	}

	return -1;
}

int waitClients() {

	pid_t id;
	int status, numberOfJob = 1;
	int cntOfExitedCl = 0;
	while (numberOfJob != 0) {
		numberOfJob = 0;
		id = waitpid(-1, &status, WNOHANG | WUNTRACED);
		switch (id) {
		case -1:
			if (errno == 10) break;
			perror("wait id");
			exit(1);
		case 0:
			continue;
		default:
			numberOfJob = findJob(id);
			switch (numberOfJob) {
			case -1:
				printf("finding error, can`t find %d\n", id);
				exit(1);
			default:
				if (close(pfds[numberOfJob].fd) == -1) {
					perror("close wait error");
					exit(1);
				}
				ids[numberOfJob].isUsing = 0;
				ids[numberOfJob].id = 0;
				pfds[numberOfJob].fd = -1;
				cntOfExitedCl++;
			}
		}
	}

	return cntOfExitedCl;
}


int findFree() {
	int curJob = 1;
	while (ids[curJob].isUsing) {
		curJob++;
		if (curJob == 20) {
			printf("Too many clients");
			for (;;) {
				if (waitClients() != 0) {
					break;
				}
			}

			return findFree();
		}
	}

	return curJob;

}

void readInfoFromClient(int fd, int pfdNum) {
	int cntOfBytesGetted = 0;
	char str[1024];
	pid_t id = 0;

	switch (id = fork()) {
	case -1:
		perror("fork error");
		exit(1);
	case 0:
		for (;;) {


			cntOfBytesGetted = read(fd, str, 1023);
			if (cntOfBytesGetted == -1) {
				perror("read error");
				exit(1);
			}
			if (cntOfBytesGetted <= 0) {
				break;
			}

			
			for (int i = 0; i < cntOfBytesGetted; i++) {
				str[i] = toupper(str[i]);
			}

			printf("%s\n", str);
		}

		if (close(fd) == -1) {
			perror("close S error");
			exit(0);
		}


		exit(0);
	default:
		ids[pfdNum].id = id;
		return;
	}

}


int main() {
	int socket_fd = 0, client_socket_fd = 0, jobid = 0;
	struct sockaddr_un address;
	socklen_t addrlen = sizeof(address);
	char path[] = "/tmp/lab31_socket";
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, path);
	int cntOfClients = 0;


	for (int i = 0; i < 20; i++) {
		ids[i].isUsing = 0;
		ids[i].id = 0;
		pfds[i].fd = -1;
		pfds[i].events = POLLIN;
	}


	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("socket error");
		exit(1);
	}



	if (unlink(path) == -1) {
		if (errno != 2) {
			perror("unlink error");
			exit(1);
		}
	}
	if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
		perror("bind error");
		exit(1);
	}

	if (listen(socket_fd, 20) == -1) {
		perror("listen error");
		exit(1);
	}

	pfds[0].fd = socket_fd;
	ids->isUsing = 1;

	for (;;) {
		switch (poll(pfds, 20, 10000)) {
		case -1:
			perror("poll error");
			exit(1);
		case 0:
			if (cntOfClients == 0) {
				exit(0);
			}
			break;
		default:
			if (pfds[0].revents & POLLIN) {
				client_socket_fd = accept(socket_fd, (struct sockaddr*)&address, &addrlen);
				pfds[0].revents = 0;
				jobid = findFree();
				pfds[jobid].fd = client_socket_fd;
				cntOfClients++;
				continue;
			}

			for (int i = 1; i < 20; i++) {
				if (pfds[i].revents & POLLIN && pfds[i].fd != -1 && ids[i].isUsing == 0) {
					pfds[i].revents = 0;
					ids[i].isUsing = 1;
					readInfoFromClient(pfds[i].fd, i);
				}
			}

			cntOfClients -= waitClients();
		}
	}

	if (close(socket_fd) == -1) {
		perror("close main error");
		exit(1);
	}
	
	if (remove(path) == -1) {
		perror("remove error");
		exit(1);
	}

	exit(0);
}