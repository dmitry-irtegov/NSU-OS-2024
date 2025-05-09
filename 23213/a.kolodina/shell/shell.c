#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <termios.h>
#include "shell.h"

char prompt[1050]; 
pid_t shell;
struct termios shell_modes;
job* bg_jobs = NULL;
int bg_jobs_number = 0;
int biggest_idx = 0;
char cwd[1025];
FILE* script = 0;
int is_interactive = 0;

int main(int argc, char *argv[]){
    if (argv[1]) {
        script = fopen(argv[1], "r");
    }
    is_interactive = isatty(STDIN_FILENO);
    shell = getpid();
    setpgid(shell, shell);
    if (is_interactive && tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error tcsetpgrp shell");
        exit(1);
    }
    if (is_interactive && tcgetattr(STDIN_FILENO, &shell_modes) == -1) {
        perror("tcgettatrr error");
        exit(1);
    }
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; 
    sigaction(SIGINT, &sa, NULL);

    char line[1024];   
    int ncmds;
    job* j;
    getcwd(cwd, sizeof(cwd));
    sprintf(prompt,"[%s] $ ", cwd);

    while (promptline(prompt, line, sizeof(line)) >= 0) { 
        monitor_jobs();
        if (is_interactive && tcsetpgrp(STDIN_FILENO, shell) == -1) {
            perror("error tcsetpgrp shell");
            exit(1);
        }
        if (!(j = parseline(line))){
            continue;  
        }
        job* curr;
        while (j) {
            curr = j;
            j = j->next;
            start_job(curr);
        }
    }
    return 0;
}

void sigint_handler(int sig){
    write(STDOUT_FILENO, "\n", strlen("\n"));
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

void fg(char* arg) {
    int idx = 0;
    int found = 0;
    job* j = bg_jobs;
    if (!j) 
        return;
    if (arg) {
        if (*arg == '%') {
            *arg++;
        }
        idx = atoi(arg);
    } else {
        idx = -1;
    }
    pid_t job_pid;
    while (j->next) {
        if (j->idx == idx){
            found = 1;
            break;
        }
        j = j->next;
    }
    if (j->idx == idx)
        found = 1;
    if (!found && idx != -1) {
        fprintf(stderr, "fg:  %d: no such job\n", idx);
        return;
    }
    j->status = 1;
    j->bg = 0;
    printf("%s\n", j->job_name);
    if (is_interactive && tcsetpgrp(STDIN_FILENO, j->job_pgid) == -1)
        perror("fg fail");
    kill(-j->job_pgid, SIGCONT);
    wait_for_job(j);
}

void bg(char* arg) {
    int idx = 0;
    int found = 0;
    job* j = bg_jobs;
    if (!j) 
        return;
    if (arg) {
        if (*arg == '%') {
            *arg++;
        }
        idx = atoi(arg);
    } else {
        idx = -1;
    }
    pid_t job_pid;
    while (j->next) {
        if (j->idx == idx){
            found = 1;
            break;
        }
        j = j->next;
    }
    if (j->idx == idx)
        found = 1;
    if (!found && idx != -1) {
        fprintf(stderr, "bg:  %d: no such job\n", idx);
        return;
    }
    j->status = 1;
    j->bg = 1;
    printf("%s\n", j->job_name);
    kill(-j->job_pgid, SIGCONT);
}

void jobs() { 
    if (!bg_jobs)
        return;
    monitor_jobs();
    int idx = 0;
    job* j = bg_jobs;
    while (j) {
        if (j->status == 1)
            printf("Running ");
        else if (j->status == 2) 
            printf("Stopped ");
        if (j->bg)
            printf("%s %d [%d]\n", j->job_name, j->job_pgid, j->idx);
        j = j->next;
        idx++;
    }
}
