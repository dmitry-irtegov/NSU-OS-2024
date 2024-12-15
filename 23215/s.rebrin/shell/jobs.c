#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
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

void look_exs() {
	for (int i = 1; i <= count; i++) if (jobs[i].status != 'f') exs++;
}

void count_to_ne_f() {
	while (jobs[count].status == 'f') count--;
}

char* getstat(char s) {
	switch (s){
		case 'r': return "Running"; 
		case 's': return "Stopped";
		case 'f': return "Finished";
		default: return "Unknown";
	}
}

void print_jobs() {
	count_to_ne_f();
	look_exs();
	printf("Job List:\n");
	printf("ID\tPID\tStatus\t\tName\n");
	printf("---------------------------------------------------\n");

	for (int i = 1; i <= count; i++) {
		char* status_desc;
		if (jobs[i].status == 'f') continue;
		switch (jobs[i].status) {
		case 'r': status_desc = "Running"; break;
		case 's': status_desc = "Stopped"; break;
		default:  status_desc = "Unknown"; break;
		}

		// Указываем статус: +, - или пустой для текущего задания
		char priority = (i == plus) ? '+' : (i == minus) ? '-' : ' ';

		printf("[%d]%c\t%d\t%-10s\t%s\n",
			i, priority, jobs[i].pid, status_desc, jobs[i].name);
	}

	if (!exs) {
		printf("No jobs available.\n");
	}
}



void print_job(char* i) {
	
	if (isdigit(i[0]) && atoi(i) <= count && (jobs[atoi(i)].status != 'f')) {
		// Указываем статус: +, - или пустой для текущего задания
		int ind = atoi(i);
		char priority = (ind == plus) ? '+' : (ind == minus) ? '-' : ' ';

		printf("[%d]%c\t%d\t%-10s\t%s\n",
			ind, priority, jobs[ind].pid, getstat(jobs[ind].status), jobs[ind].name);
	}
	else if (i[0] == '+') {
		printf("[%d]%c\t%d\t%-10s\t%s\n",
			plus, '+', jobs[plus].pid, getstat(jobs[plus].status), jobs[plus].name);
	}
	else if (i[0] == '-') {
		printf("[%d]%c\t%d\t%-10s\t%s\n",
			minus, '-', jobs[minus].pid, getstat(jobs[minus].status), jobs[minus].name);
	}
	else {
		printf("jobs: %s: No such jobs.\n", i);
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
	if (fr != 0 && jobs[fr].status == 'f') {
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

void upd_job() {
	int status;
	for (int i = 1; i <= count; i++) {
		if (jobs[i].status == 'f') continue;
		pid_t result = waitpid(jobs[i].pid, &status, WNOHANG /* | WUNTRACED | WCONTINUED*/);

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
			if (errno == ECHILD) {
			}
			else {
				perror("waitpid");
				fprintf(stderr, "- [%d]", i);
			}
		}
	}

	if (!exs) {
		clear_jobs();
		init_jobs();
	}
}


int to_fg(int job_id) {
	upd_job();
	fr = job_id;
	printf("((%d))", job_id);

	if (job_id > count || jobs[job_id].status == 'f') {
		perror("shell: no such job");
		return -1;
	}


	// Ожидание завершения процесса
	int status;
	pid_t code = waitpid(jobs[job_id].pid, &status, WUNTRACED);
	if (code == -1) {
		perror("unable to wait termination of one of job processes");
	}
	jobs[job_id].status = WIFSTOPPED(status) ? 's' : 'f';


	upd_job();

	if (tcgetattr(0, &jobs[job_id].term) == -1) {
		perror("error");
	}

	if (tcsetpgrp(0, getpid()) == -1) {
		perror("Unable to set process to fg");
		return -1;
	}

	fr = 0;
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

void pr_job(pid_t pid) {
	int i;
	for (i = count; i > 0 && (jobs[i].status == 'f' || jobs[i].pid != pid); i--);
	char str[20];
	sprintf(str, "%d", i);
	if (i) print_job(str);
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

void clear_j() {
	free(jobs);
}