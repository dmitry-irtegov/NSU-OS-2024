#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>

unsigned int count = 0;
unsigned int cup = 20;
unsigned int plus = 0;
unsigned int minus = 0;
unsigned int fr = 0;
unsigned int exs = 0;

typedef struct {
	char name[100];
	char status;
	pid_t pid;
	struct termios term;

} job;

job* jobs;
struct termios shell_term;

void init_jobs() {
	jobs = (job*)malloc(sizeof(job) * cup);
}

void print_jobs() {
	printf("Job List:\n");
	printf("ID\tPID\tStatus\t\tName\n");
	printf("---------------------------------------------------\n");

	for (int i = 1; i <= count; i++) {
		char* status_desc;

		switch (jobs[i].status) {
		case 'r': status_desc = "Running"; break;
		case 's': status_desc = "Stopped"; break;
		case 'f': status_desc = "Finished"; break;
		default:  status_desc = "Unknown"; break;
		}

		// Указываем статус: +, - или пустой для текущего задания
		char priority = (i == plus) ? '+' : (i == minus) ? '-' : ' ';

		printf("[%d]%c\t%d\t%-10s\t%s\n",
			i, priority, jobs[i].pid, status_desc, jobs[i].name);
	}

	if (count == 0) {
		printf("No jobs available.\n");
	}
}

void reorder_priorities() {
	// Проверка, если текущее задание завершилось
	if (jobs[plus].status == 'f') {
		// Предыдущее задание становится текущим
		plus = minus;

		// Ищем следующее незавершенное задание для обновления minus
		int m = minus;
		for (int i = 1; i <= count; i++) {
			if (i != plus && jobs[i].status != 'f') {
				minus = i;
				break;
			}
		}
	}
	else if (jobs[minus].status == 'f') {
		for (int i = 1; i <= count; i++) {
			if (i != plus && jobs[i].status != 'f') {
				minus = i;
				break;
			}
		}
	}

	// Если задание на переднем плане (`fr`) завершено
	if (fr != -1 && jobs[fr].status == 'f') {
		fr = 0; // Сбрасываем индекс переднего плана
	}
}


int add_job(char* name, pid_t pid, int frnt) {
	if (count + 1 == 0) {
		return 0;
	}

	exs++;
	if (count >= cup) {
		cup *= 2;
		jobs = (job*)realloc(jobs, cup * sizeof(job));
	}

	count++;
	strcpy(jobs[count].name, name);
	jobs[count].pid = pid;
	jobs[count].status = 'r';  // Запущен

	if (frnt) {
		reorder_priorities();
		minus = plus;
		plus = count;
	}
	else {
		if (!plus) plus = count;
		minus = count;
	}

	return count;
}

void upd_job() {
	int status;
	for (int i = 1; i <= count; i++) {
		if (jobs[i].status == 'f') continue;
		pid_t result = waitpid(jobs[i].pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

		if (result == 0) {
			continue;
		}
		else if (result > 0) {
			char stat = (i == plus) ? '+' : (i == minus) ? '-' : ' ';

			if (WIFEXITED(status)) {
				exs--;
				jobs[i].status = 'f'; // завершён
				printf("[%d]%c (%s) finished, exit: %d\n", i, stat, jobs[i].name, WEXITSTATUS(status));
			}
			else if (WIFSTOPPED(status)) {
				jobs[i].status = 's'; // остановлен
				printf("[%d]%c (%s) stopped signal %d\n", i, stat, jobs[i].name, WSTOPSIG(status));
			}
			else if (WIFCONTINUED(status)) {
				jobs[i].status = 'r'; // возобновлен
				printf("[%d]%c (%s) resumed\n", i, stat, jobs[i].name);
			}
			else if (WIFSIGNALED(status)) {
				exs--;
				jobs[i].status = 'f'; // завершён сигналом
				printf("[%d]%c (%s) killed by signal %d\n", i, stat, jobs[i].name, WTERMSIG(status));
			}
		}
		else {
			perror("waitpid");
		}
	}

	if (!exs){
		clear_jobs();
		init_jobs();
	}
}

void clear_jobs() {
	free(jobs);
	init_jobs();
	count = 0;
	cup = 20;
	plus = 0;
	minus = 0;
	fr = 0;
	exs = 0;
}

int to_fg(int job_id) {
	print_jobs();
	upd_job();
	fr = job_id;
	printf("((%d))", job_id);

	if (job_id > count || jobs[job_id].status == 'f') {
		perror("shell: no such job");
		return -1;
	}

	fprintf(stderr, "hereeeee");


	// Ожидание завершения процесса
	siginfo_t info;
	pid_t code = waitid(P_PID, jobs[job_id].pid, &info, WEXITED);
	if (code == -1) {
		perror("unable to wait termination of one of job processes");
	}
	fprintf(stderr, "hereeeeerrrrr");

	upd_job();

	if (tcgetattr(0, &jobs[job_id].term) == -1) {
		perror("error");
	}

	if (tcsetpgrp(0, getpid()) == -1) {
		perror("Unable to set process to fg");
		return -1;
	}
	reorder_priorities();

	return 0;
}

int to_bg(int job_id) {
	upd_job();

	if (job_id > count || jobs[job_id].status == 'f') {
		perror("shell: no such job");
		return -1;
	}

	if (jobs[job_id].status == 's') {
		kill(jobs[job_id].pid, SIGCONT);
	}

	reorder_priorities();

	return 0;
}

pid_t get_g_int(int job_id) {
	if (0 < job_id && job_id <= count && jobs[job_id].status != 'f') {
		return jobs[job_id].pid;
	}
	else {
		return 0;
	}
}

pid_t get_g_ch(char job_id) {
	switch (job_id) {
	case '-':
		return (minus && jobs[minus].status != 'f') ? jobs[minus].pid : 0;
	case '+':
		return (plus && jobs[plus].status != 'f') ? jobs[plus].pid : 0;
	case '%':  // Обработка текущего задания (например, `%`)
		return (plus && jobs[plus].status != 'f') ? jobs[plus].pid : 0;
	default:
		return -1;
	}
}