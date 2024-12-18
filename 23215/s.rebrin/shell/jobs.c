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
#include <signal.h>
#include "shell.h"

unsigned int count = 0;//Количество всех джоб
unsigned int cup = 20;//Для вектора
unsigned int plus = 0;//индекс для +
unsigned int minus = 0;//-
unsigned int exs = 0; //количество незавершенных
pid_t shell_id;

typedef struct {
	char name[100];
	char status;
	pid_t pid;
	struct termios term;

} job;

job* jobs;
struct termios shell_term;//Терм шелла
struct termios saved_term;//Образец терма

void set_shell_id(pid_t t) {
	shell_id = t;
}

void init_jobs() {
	jobs = (job*)malloc(sizeof(job) * cup);
}

//Проверить все рабочие джобы
void look_exs() {
	exs = 0;
	for (int i = 1; i <= count; i++) if (jobs[i].status != 'f') exs++;
}

//Снизить count до последней не завершенной
void count_to_ne_f() {
	while (count && jobs[count].status == 'f') count--;
}

int get_job_id(pid_t p) {
	for (int i = 0;i <= count;i++) {
		if (jobs[count].pid == p) return count;
	}
	return -1;
}

//Статус для вывода
char* getstat(char s) {
	switch (s) {
	case 'r': return "Running";
	case 's': return "Stopped";
	case 'f': return "Finished";
	default: return "Unknown";
	}
}

//Распечатать все джобы
void print_jobs() {
	upd_job();
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

//Настроить saved_term
void set_default_termios() {
	tcgetattr(STDIN_FILENO, &saved_term);
	tcgetattr(STDIN_FILENO, &shell_term);
}

//Сбросить терм через saved_term
void return_termios(struct termios* term) {
	memcpy(term, &saved_term, sizeof(struct termios));
}

//распечатать одну джобу
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

//Распределить + и -
//По умолчанию jobs[count]  -  + или -
void reorder_priorities(int last) {

	if (!count || !exs) {
		clear_jobs();
		init_jobs();
		return;
	}

	if (!last) {
		if (jobs[plus].status == 'f') {
			if (jobs[minus].status == 'f') {
				plus = count;
				minus = count - 1;
				while (minus && jobs[minus].status == 'f') minus--;
			}
			else {
				plus = minus;
				if (count != 1) minus = count;
				else minus = 0;
			}
		}
		else if (jobs[minus].status == 'f') {
			if (plus == count) {
				minus = count - 1;
				while (minus && jobs[minus].status == 'f') minus--;
			}
			else {
				minus = count;
			}
		}
	}
	else {
		if (last <= count && jobs[last].status != 'f') return;
		else {
			plus = last;
			if (plus == count) {
				minus = count - 1;
				while (minus && jobs[minus].status == 'f') minus--;
			}
			else {
				minus = count;
			}
		}
	}
}

//Добавить джобу
int add_job(char* name, pid_t pid, int frnt) {
	upd_job();

	exs++;
	if (count >= cup) {
		cup *= 2;
		jobs = (job*)realloc(jobs, cup * sizeof(job));
	}

	count++;
	strcpy(jobs[count].name, name);
	jobs[count].pid = pid;
	jobs[count].status = 'r';  // Запущен
	return_termios(&jobs[count].term);

	if (frnt) {
		reorder_priorities(count);
		minus = plus;
		plus = count;
	}
	else {
		if (!plus) plus = count;
		minus = count;
	}

	return count;
}

//Все сбросить
void clear_jobs() {
	free(jobs);
	count = 0;
	cup = 20;
	plus = 0;
	minus = 0;
	exs = 0;
}

//Обновить все джобы
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
			if (errno != ECHILD) {
				perror("waitpid");
				fprintf(stderr, "- [%d]", i);
			}
		}
	}

	count_to_ne_f();

	look_exs();

	reorder_priorities(0);

}

//Вывести в fg
int to_fg(int job_id_h) {
	upd_job();
	int job_id;

	if (job_id_h < 0 || job_id_h > count || jobs[job_id_h].status == 'f') {
		fprintf(stderr, "shell: no such job");
		return -1;
	}

	if (job_id_h) job_id = job_id_h;
	else job_id = plus;

	if (jobs[job_id].status == 's') kill(jobs[job_id].pid, SIGCONT);

	// Ожидание завершения процесса
	int status;
	signal(SIGCHLD, SIG_DFL);//Игнорируем всех детей пока

	/*if (tcsetpgrp(0, jobs[job_id].pid) == -1) {
		perror("Unable to set process to fg");
	}*/

	/*if (tcsetattr(0, TCSADRAIN, &jobs[job_id].term) == -1) {
		perror("Job set attr");
	}*/

	pid_t code = waitpid(jobs[job_id].pid, &status, WUNTRACED);
	if (code == -1) {
		perror("unable to wait termination of one of job processes");
	}
	jobs[job_id].status = WIFSTOPPED(status) ? 's' : 'f';

	upd_job();
	signal(SIGCHLD, sigCHLD);

	/*if (jobs[job_id].status != 'f' && tcgetattr(0, &jobs[job_id].term) == -1) {
		perror("error");
	}*/

	/*if (tcsetpgrp(0, shell_id) == -1) {
		perror("Unable to set process to fg");
		return -1;
	}*/

	if (tcsetattr(0, TCSADRAIN, &shell_term) == -1) {
		perror("Shell set attr");
	}

	return 0;
}

//В bg
int to_bg(int job_id) {
	upd_job();

	if (job_id > count || jobs[job_id].status == 'f') {
		perror("shell: no such job");
		return -1;
	}

	if (jobs[job_id].status == 's') {
		kill(jobs[job_id].pid, SIGCONT);
	}

	return 0;
}

//Получить pid по job id
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

//Распечатать по pid
void pr_job(pid_t pid) {
	upd_job();
	int i;
	for (i = count; i > 0 && (jobs[i].status == 'f' || jobs[i].pid != pid); i--);
	char str[20];
	sprintf(str, "%d", i);
	if (i) print_job(str);
}

