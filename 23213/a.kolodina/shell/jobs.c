#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include "shell.h"

extern int is_interactive;
#define JOB_NAME_SIZE 256

void build_job(job* j, struct command* cmds, int curr_idx, int ncmds) {
    if (ncmds == 0 || !cmds->cmdargs)
        return;
    j->job_name = (char*) malloc(JOB_NAME_SIZE * sizeof(char));
    if (j->job_name == NULL) {
        perror("malloc error");
        exit(1);
    }
    memset(j->job_name, '\0', JOB_NAME_SIZE * sizeof(char));

    process* head_p = (process*) malloc(sizeof(process));
    if (head_p == NULL) {
        perror("malloc error");
        exit(1);
    }
    j->processes = head_p;
    j->job_pgid = 0;
    j->idx = 0;
    int flag = 0;
    process* curr_p = head_p;
    for (int i = curr_idx; i < ncmds; i++) {
        if (j->bg == 1) 
            curr_p->bg = 1;
        int idx = 0;
        curr_p->argv.cmdflag = cmds[i].cmdflag;
        curr_p->prev_fildes[0] = 0;
        curr_p->prev_fildes[1] = 0;
        while (cmds[i].cmdargs[idx] && strlen(j->job_name) < JOB_NAME_SIZE) {
            curr_p->appfile = j->appfile;
            curr_p->outfile = j->outfile;
            curr_p->infile = j->infile;
            curr_p->argv.cmdargs[idx] = strdup(cmds[i].cmdargs[idx]);
            if (curr_p->argv.cmdargs[idx] == NULL) {
                perror("strdup failed");
                exit(1);
            }
            if ((curr_p->argv.cmdflag == INPIP || curr_p->argv.cmdflag == 3) && idx == 0)
                j->job_name = strncat(j->job_name, "| ", JOB_NAME_SIZE - strlen(j->job_name));

            j->job_name = strncat(j->job_name, cmds[i].cmdargs[idx], JOB_NAME_SIZE - strlen(j->job_name));
            j->job_name = strncat(j->job_name, " ", JOB_NAME_SIZE - strlen(j->job_name));

            if (curr_p->outfile && (i == ncmds - 1) && !cmds[i].cmdargs[idx+1]) {
                j->job_name = strncat(j->job_name, "> ", JOB_NAME_SIZE - strlen(j->job_name));
                j->job_name = strncat(j->job_name, curr_p->outfile, JOB_NAME_SIZE - strlen(j->job_name));
            }
            if (curr_p->appfile && (i == ncmds - 1) && !cmds[i].cmdargs[idx+1]) {
                j->job_name = strncat(j->job_name, ">> ", JOB_NAME_SIZE - strlen(j->job_name));
                j->job_name = strncat(j->job_name, curr_p->appfile, JOB_NAME_SIZE - strlen(j->job_name));
            }
            if (curr_p->infile) {
                j->job_name = strncat(j->job_name, "< ", JOB_NAME_SIZE - strlen(j->job_name));
                j->job_name = strncat(j->job_name, curr_p->infile, JOB_NAME_SIZE - strlen(j->job_name));
            }
            idx++;
        }
        curr_p->argv.cmdargs[idx] = NULL;
        if (i < ncmds -1) {
            curr_p->next = (process*) malloc(sizeof(process));
            if (curr_p->next == NULL) {
                perror("malloc error");
                exit(1);
            }
            curr_p->next->prev = curr_p;
            curr_p = curr_p->next;
        } else {
            curr_p->next = NULL;
        }
    }
}

void start_process(process* p) {
    p->status = 1;
    if (p->bg == 0) {
        if (is_interactive && tcsetpgrp(STDIN_FILENO, p->g_pid) == -1) { 
            perror("error setting terminal child");
            exit(1);
        } 
    }
    if (p->infile){
        int in_desc = open(p->infile, O_RDONLY);
        if (in_desc == -1) {
            perror("error while opening descriptor to read");
            exit(1);
        }
        if (dup2(in_desc, STDIN_FILENO) == -1) {
            perror("redirecting stdin error");
            exit(1);
        }
        close(in_desc);
    }
    if (p->outfile) { 
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int out_desc = open(p->outfile, O_WRONLY | O_CREAT | O_TRUNC, mode);
        if (out_desc == -1) {
            perror("error while opening descriptor to write");
            exit(1);
        }
        if (dup2(out_desc, STDOUT_FILENO) == -1) {
            perror("redirecting stdout error");
            exit(1);
        }
        close(out_desc);
    }
    if (p->appfile) { 
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int app_desc = open(p->appfile, O_APPEND | O_WRONLY | O_CREAT, mode);
        if (app_desc == -1) {
            perror("error while opening descriptor to append");
            exit(1);
        }
        if (dup2(app_desc, STDOUT_FILENO) == -1) {
            perror("redirecting stdout error");
            exit(1);
        }
        close(app_desc);
    }

    if (p->argv.cmdflag == INPIP || p->argv.cmdflag == 3) { 
        if (dup2(p->prev_fildes[0], STDIN_FILENO) == -1) {
            perror("redirecting stdin pipe error"); 
            exit(1);
        }
        close(p->prev_fildes[0]); 
        if (p->argv.cmdflag != 3) 
            close(p->prev_fildes[1]);
    }
    if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) { 
        if (dup2(p->fildes[1], STDOUT_FILENO) == -1) {
            perror("redirecting stdout pipe error");
            exit(1);
        }
        close(p->fildes[0]);
        close(p->fildes[1]);
    }
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    if (setpgid(p->p_pid, p->g_pid)== -1) {
        perror("setpgid error child");
        exit(1);
    } 
    execvp(p->argv.cmdargs[0], p->argv.cmdargs);
    perror("execvp error");
    exit(1);
}

void start_job(job* curr_job) {
    if (strcmp(curr_job->processes->argv.cmdargs[0], "fg") == 0) {
        fg(curr_job->processes->argv.cmdargs[1]);
        remove_job(curr_job);
        free_job(curr_job);
        return;
    }
    if (strcmp(curr_job->processes->argv.cmdargs[0], "bg") == 0) {
        bg(curr_job->processes->argv.cmdargs[1]);
        remove_job(curr_job);
        free_job(curr_job);
        return;
    }
    if (strcmp(curr_job->processes->argv.cmdargs[0], "jobs") == 0) {
        jobs();
        remove_job(curr_job);
        free_job(curr_job);
        return;
    }
    curr_job->status = 1;
    for (process* p = curr_job->processes; p; p = p->next) {
        if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) { 
            if (pipe(p->fildes) == -1) {
                perror("pipe error");
                exit(1);
            }
            p->next->prev_fildes[0] = p->fildes[0];
            p->next->prev_fildes[1] = p->fildes[1];
        }
        pid_t pid;
        pid = fork(); 
        if (pid == -1) {
            perror("fork error");
            exit(1);
        } else if (pid == 0) { 
            p->p_pid = getpid();
            if (curr_job->job_pgid == 0) 
                curr_job->job_pgid = p->p_pid;
            p->g_pid = curr_job->job_pgid;
            start_process(p);
        } else {
            p->p_pid = pid;
            if (curr_job->job_pgid == 0) {
                curr_job->job_pgid = p->p_pid; 
                p->g_pid = curr_job->job_pgid; 
                if (setpgid(p->p_pid, curr_job->job_pgid) == -1)
                    perror("setpgid error parent");
                if (curr_job->bg == 0) { 
                    if (is_interactive && tcsetpgrp(STDIN_FILENO, curr_job->job_pgid) == -1) 
                        perror("child getting terminal error");
                }
            }
            p->g_pid = curr_job->job_pgid; 
            if (setpgid(p->p_pid, curr_job->job_pgid) == -1) 
                perror("setpgid error parent");

            if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) {
                close(p->fildes[1]);
                p->next->prev_fildes[0] = p->fildes[0];
            }
            if (p->prev_fildes[0] != 0) 
                close(p->prev_fildes[0]);
        }
    }

    if (curr_job->bg == 1) { 
        if (!bg_jobs) {
            bg_jobs = curr_job;  
            curr_job->next = NULL;
        } else {
            job* j = bg_jobs;
            while (j->next) {
                j = j->next;
            }
            j->next = curr_job;
            curr_job->prev = j;
            curr_job->next = NULL;
        }
        bg_jobs_number++;
        biggest_idx++;
        curr_job->idx = biggest_idx;
        fprintf(stderr, "[%d] %d \n", curr_job->idx, curr_job->job_pgid);
    }
    if (curr_job->bg == 0) {
        wait_for_job(curr_job);
    }
    if (is_interactive && tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error setting terminal to shell");
        exit(1);
    }
    if (is_interactive && tcsetattr (STDIN_FILENO, TCSADRAIN, &shell_modes) == -1) {
        perror("tcsetattr error");
        exit(1);
    }
}

void wait_for_job(job* curr_job) {
    if (!curr_job) {
        return;
    }
    int flag = 0;
    int stop_notified = 0;
    int wstatus;
    for (process* p = curr_job->processes; p; p = p->next) {
        int wstatus;
        if (waitpid(p->p_pid, &wstatus, WUNTRACED) == -1) {
            perror("waitpid error");
            exit(1);
        }
        if (WIFEXITED(wstatus)) {
            flag = 1;
        }
        if (WIFSIGNALED(wstatus)) {
            flag = 1;
        }
        if (WIFSTOPPED(wstatus)) {
            curr_job->status = 2;
            curr_job->bg = 1;
            int flag2 = 0;
            job* j = bg_jobs;
            if (!bg_jobs) {
                curr_job->next = NULL;
                bg_jobs = curr_job;
                bg_jobs_number++;  
                biggest_idx++;
                curr_job->idx = biggest_idx;
            } else {
                while (j->next) {
                    if (j->job_pgid == curr_job->job_pgid) {
                        j->status = 2; 
                        flag2 = 1; 
                        break;
                    }
                    j = j->next;
                }
                if (j->job_pgid == curr_job->job_pgid) {
                    j->status = 2; 
                    flag2 = 1; 
                }
                if (flag2 == 0) {
                    j->next = curr_job;
                    curr_job->prev = j;
                    curr_job->next = NULL;
                    bg_jobs_number++;
                    biggest_idx++;
                    curr_job->idx = biggest_idx;
                }
            }
        }
    }
    if (curr_job->status == 2 && !stop_notified) {
        stop_notified = 1;
        fprintf(stderr, "[%d] stopped %s \n", curr_job->idx, curr_job->job_name);
    }
    if (flag) {
        if (curr_job->idx == biggest_idx && curr_job->idx != 0) {
            biggest_idx--; 
        }
        remove_job(curr_job);
        free_job(curr_job);
    }
    if (is_interactive && tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error setting terminal to shell");
        exit(1);
    }

    if (is_interactive && tcsetattr (STDIN_FILENO, TCSADRAIN, &shell_modes) == -1) {
        perror("tcsetattr error");
        exit(1);
    }
}

void free_process(process* p){
    if (!p)
        return;
    int i = 0;
    while (p->argv.cmdargs[i]) {
        free(p->argv.cmdargs[i]);
        i++;
    }
    free(p);
}

void free_job(job* j){
    if (!j)
        return;
    process* p = j->processes;
    while (p) {
        process* next = p->next;
        free_process(p);
        p = next;
    }
    free(j->job_name);
    free(j);
}

job* find_job(pid_t pid){ 
    if (!bg_jobs)
        return NULL;
    job* j = bg_jobs;

    pid_t job_id = getpgid(pid);
    while (j) {
        if (job_id != -1) { 
            if (j->job_pgid == job_id) {
                return j;
            }
        } else { 
            for (process* p = j->processes; p; p = p->next) {
                if (p->p_pid == pid) {
                    return j;
                }
            }
        }
        j = j->next;
    }
    return NULL;
}

void monitor_jobs() {
    if (!bg_jobs) {
        biggest_idx = 0;
        return;
    }
    job* j;
    int wstatus;
    pid_t process_pid;
    pid_t job_id;
    while ((process_pid = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED | WNOHANG))) {
        j = find_job(process_pid);
        int notified = 0;
        if (j) {
            if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)) {
                if (!notified){
                    printf("[%d] Done   %s\n", j->idx, j->job_name);
                    if (j->idx != 0 && j->idx == biggest_idx) 
                        biggest_idx--;
                    notified = 1;
                }
                j->status = 0;
                remove_job(j);
                free_job(j);
                bg_jobs_number--;
            }
            if (WIFSTOPPED(wstatus)) {
                j->status = 2;
            }
            if (WIFCONTINUED(wstatus)) {
                j->status = 1;
            }
        }
        if (process_pid == -1) 
            return;
    }
}

void remove_job(job* j) { 
    if (!j)
        return;
    if (j->prev) {
        j->prev->next = j->next;
    }
    if (bg_jobs == j) {
            if (j->next && j->next->bg == 1) {
                bg_jobs = j->next;
            } else {
                bg_jobs = NULL;
            }
        }
    if (j->next)
        j->next->prev = j->prev;
}
