/**
 * @file jobs.c
 * @brief Job control implementation for shell
 *
 * This file implements job control functionality including:
 * - Job creation and management
 * - Process control and monitoring
 * - Foreground/background job handling
 * - Terminal control
 * - Signal handling for job control
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include "builtins.h"
#include "shell.h"
#include "jobs.h"

extern int is_interactive;
#define JOB_NAME_SIZE 256

/**
 * @brief Build a job structure from parsed commands
 * 
 * Creates a new job structure and initializes it with processes
 * based on parsed commands. Handles:
 * - Process creation and linking
 * - Command argument copying
 * - File redirection setup
 * - Job name construction
 * 
 * @param j Job structure to build
 * @param cmds Array of parsed commands
 * @param curr_idx Starting index in cmds array
 * @param ncmds Number of commands to process
 */
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
            curr_p->errfile = j->errfile;  
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
            if (curr_p->errfile && (i == ncmds - 1) && !cmds[i].cmdargs[idx+1]) {
                j->job_name = strncat(j->job_name, "2> ", JOB_NAME_SIZE - strlen(j->job_name));
                j->job_name = strncat(j->job_name, curr_p->errfile, JOB_NAME_SIZE - strlen(j->job_name));
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

/**
 * @brief Start execution of a process
 * 
 * Handles:
 * - Terminal control for foreground processes
 * - Process status management
 * - File descriptor setup for pipes and redirections
 * - Signal handling
 * - Command execution
 * 
 * @param p Process to start
 */
void start_process(process* p) {
    if (p->bg == 0) {
        if (is_interactive && tcsetpgrp(STDIN_FILENO, p->g_pid) == -1) { 
            perror("error setting terminal child");
            exit(1);
        } 
    }
    
    p->status = 1;  // Set process status
    
    // Redirect input from file
    if (p->infile) {
        int in_desc = open(p->infile, O_RDONLY);
        if (in_desc == -1) {
            perror("error while opening descriptor to read");
            exit(1);
        }
        if (dup2(in_desc, STDIN_FILENO) == -1) {
            close(in_desc);
            perror("redirecting stdin error");
            exit(1);
        }
        close(in_desc);
    }
    
    // Redirect output to file
    if (p->outfile) {
        int out_desc = open(p->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_desc == -1) {
            perror("error while opening descriptor to write");
            exit(1);
        }
        if (dup2(out_desc, STDOUT_FILENO) == -1) {
            close(out_desc);
            perror("redirecting stdout error");
            exit(1);
        }
        close(out_desc);
    }
    
    // Redirect stderr to file
    if (p->errfile) {
        int err_desc = open(p->errfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (err_desc == -1) {
            perror("error while opening descriptor for stderr");
            exit(1);
        }
        if (dup2(err_desc, STDERR_FILENO) == -1) {
            close(err_desc);
            perror("redirecting stderr error");
            exit(1);
        }
        close(err_desc);
    }
    
    // Redirect output to file with append
    if (p->appfile) {
        int app_desc = open(p->appfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (app_desc == -1) {
            perror("error while opening descriptor to append");
            exit(1);
        }
        if (dup2(app_desc, STDOUT_FILENO) == -1) {
            close(app_desc);
            perror("redirecting stdout error");
            exit(1);
        }
        close(app_desc);
    }
    
    // Redirect input from pipe
    if (p->prev_fildes[0] != 0) {
        if (dup2(p->prev_fildes[0], STDIN_FILENO) == -1) {
            close(p->prev_fildes[0]);
            perror("redirecting stdin error");
            exit(1);
        }
        close(p->prev_fildes[0]);
    }
    
    // Redirect output to pipe
    if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) {
        if (dup2(p->fildes[1], STDOUT_FILENO) == -1) {
            close(p->fildes[1]);
            perror("redirecting stdout error");
            exit(1);
        }
        close(p->fildes[1]);
    }
    
    execvp(p->argv.cmdargs[0], p->argv.cmdargs);
    perror("execvp error");
    exit(1);
}

/**
 * @brief Start execution of a job
 * 
 * Handles:
 * - Job status management
 * - Process creation and linking
 * - Terminal control
 * - Signal handling
 * 
 * @param curr_job Job to start
 */
void start_job(job* curr_job) {
    if (!curr_job) return;
    
    // Check for built-in commands
    if (curr_job->processes && curr_job->processes->argv.cmdargs[0]) {
        const char *cmd = curr_job->processes->argv.cmdargs[0];
        if (strcmp(cmd, "fg") == 0) {
            fg(curr_job->processes->argv.cmdargs + 1);  // Skip the command itself
            remove_job(curr_job);
            free_job(curr_job);
            return;
        }
        if (strcmp(cmd, "bg") == 0) {
            bg(curr_job->processes->argv.cmdargs + 1);  // Skip the command itself
            remove_job(curr_job);
            free_job(curr_job);
            return;
        }
        if (strcmp(cmd, "jobs") == 0) {
            jobs();
            remove_job(curr_job);
            free_job(curr_job);
            return;
        }
        
        int builtin_result = is_builtin(curr_job->processes->argv.cmdargs, prompt);
        if (builtin_result != 0) {
            if (builtin_result == 1) {
                remove_job(curr_job);
                free_job(curr_job);
            }
            return;
        }
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
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            
            p->p_pid = getpid();
            if (curr_job->job_pgid == 0) 
                curr_job->job_pgid = p->p_pid;
            p->g_pid = curr_job->job_pgid;
            
            if (setpgid(0, curr_job->job_pgid) == -1)
                perror("setpgid error child");
                
            if (curr_job->bg == 0 && is_interactive) {
                if (tcsetpgrp(STDIN_FILENO, curr_job->job_pgid) == -1)
                    perror("tcsetpgrp error child");
            }
            
            start_process(p);
            exit(1);
        } else {
            p->p_pid = pid;
            if (curr_job->job_pgid == 0) {
                curr_job->job_pgid = p->p_pid; 
                p->g_pid = curr_job->job_pgid; 
                if (setpgid(p->p_pid, curr_job->job_pgid) == -1)
                    perror("setpgid error parent");
                if (curr_job->bg == 0 && is_interactive) { 
                    if (tcsetpgrp(STDIN_FILENO, curr_job->job_pgid) == -1) 
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
            curr_job->prev = NULL;
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
        printf("[%d] %d\n", curr_job->idx, curr_job->job_pgid);
    } else {
        wait_and_handle_job_completion(curr_job);
        
    }
}

/**
 * @brief Find the current job with '+' marker
 * 
 * @return job* Job with '+' marker or NULL if not found
 */
job* find_plus_job() {
    if (!bg_jobs) return NULL;

    job* curr = bg_jobs;
    job* last_stopped = NULL;
    job* last_job = NULL;

    // Iterate through all jobs and find:
    // - last stopped job
    // - last job
    while (curr) {
        if (curr->status == 2) {
            last_stopped = curr;
        }
        last_job = curr;
        curr = curr->next;
    }

    // Priority is given to stopped job
    return last_stopped ? last_stopped : last_job;
}

/**
 * @brief Find job by index from argument
 * 
 * @param arg Argument string (can start with %)
 * @return job* Found job or NULL
 */
static job* find_job_by_arg(const char* arg) {
    if (!arg) {
        return find_plus_job();
    }
    
    const char *ptr = arg;
    if (*ptr == '%') {
        ptr++;
    }
    int idx = atoi(ptr);
    
    job* j = bg_jobs;
    while (j) {
        if (j->idx == idx)
            break;
        j = j->next;
    }
    return j;
}

/**
 * @brief Check if job can be put in background
 * 
 * @param j Job to check
 * @return int 0 if job can be put in background, 1 otherwise
 */
static int check_bg_job_state(job* j) {
    if (!j) {
        printf("bg: no such job\n");
        return 1;
    }
    
    if (j->status == 1) {
        printf("bg: job already in background\n");
        return 1;
    }
    
    return 0;
}

/**
 * @brief Continue job execution in background
 * 
 * @param j Job to continue
 */
static void continue_job_in_background(job* j) {
    j->status = 1;
    j->bg = 1;
    printf("[%d] %s &\n", j->idx, j->job_name);
    kill(-j->job_pgid, SIGCONT);
}

/**
 * @brief Put a stopped job in background
 * 
 * @param arg Command arguments (can be NULL or job index)
 */
void bg(char **arg) {
    if (!bg_jobs) {
        printf("bg: no such job\n");
        return;
    }
    
    job* j = find_job_by_arg(arg[0]);
    
    if (check_bg_job_state(j) != 0) {
        return;
    }
    
    continue_job_in_background(j);
}

/**
 * @brief Collect all active jobs into array
 * 
 * @param active_jobs Array to store active jobs
 * @return int Number of active jobs found
 */
static int collect_active_jobs(job* active_jobs[]) {
    int n_active = 0;
    job* curr = bg_jobs;
    
    while (curr) {
        active_jobs[n_active++] = curr;
        curr = curr->next;
    }
    
    return n_active;
}

/**
 * @brief Find last and previous stopped jobs
 * 
 * @param active_jobs Array of active jobs
 * @param n_active Number of active jobs
 * @param last_stopped Pointer to store last stopped job
 * @param prev_stopped Pointer to store previous stopped job
 */
static void find_stopped_jobs(job* active_jobs[], int n_active, 
                            job** last_stopped, job** prev_stopped) {
    *last_stopped = NULL;
    *prev_stopped = NULL;
    
    for (int i = n_active - 1; i >= 0; i--) {
        if (active_jobs[i]->status == 2) {
            if (!*last_stopped) {
                *last_stopped = active_jobs[i];
            } else if (!*prev_stopped) {
                *prev_stopped = active_jobs[i];
                break;
            }
        }
    }
}

/**
 * @brief Find last running job
 * 
 * @param active_jobs Array of active jobs
 * @param n_active Number of active jobs
 * @return job* Last running job or NULL
 */
static job* find_last_running_job(job* active_jobs[], int n_active) {
    for (int i = n_active - 1; i >= 0; i--) {
        if (active_jobs[i]->status == 1) {
            return active_jobs[i];
        }
    }
    return NULL;
}

/**
 * @brief Determine plus and minus markers for jobs
 * 
 * @param last_stopped Last stopped job
 * @param prev_stopped Previous stopped job
 * @param last_running Last running job
 * @param last_job Last job in list
 * @param plus_job Pointer to store job with plus marker
 * @param minus_job Pointer to store job with minus marker
 */
static void determine_job_markers(job* last_stopped, job* prev_stopped,
                                job* last_running, job* last_job,
                                job** plus_job, job** minus_job) {
    if (last_stopped) {
        // If there are stopped jobs:
        *plus_job = last_stopped;  // last stopped gets '+'
        if (prev_stopped) {
            *minus_job = prev_stopped;  // previous stopped gets '-'
        } else if (last_running) {
            *minus_job = last_running;  // else last running gets '-'
        }
    } else if (last_running) {
        // If no stopped jobs but there are running jobs:
        *plus_job = last_running;  // last running gets '+'
        // No minus marker in this case
    } else {
        // If no stopped or running jobs:
        *plus_job = last_job;  // last job gets '+'
    }
}

/**
 * @brief Print job information with markers
 * 
 * @param j Job to print
 * @param plus_job Job with plus marker
 * @param minus_job Job with minus marker
 */
static void print_job_info(job* j, job* plus_job, job* minus_job) {
    char marker = ' ';
    if (j == plus_job) marker = '+';
    else if (j == minus_job) marker = '-';
    
    printf("[%d]%c  ", j->idx, marker);
    
    if (j->status == 1) {
        printf("Running                 ");
    } else if (j->status == 2) {
        printf("Stopped                 ");
    } else {
        printf("Done                    ");
    }
    
    // Add '&' for background processes
    if (j->bg) {
        printf("%s &\n", j->job_name);
    } else {
        printf("%s\n", j->job_name);
    }
}

/**
 * @brief Display information about all background jobs
 */
void jobs() { 
    if (!bg_jobs)
        return;
    monitor_jobs();

    // Collect active jobs
    job* active_jobs[50];  // Assume no more than 50 jobs
    int n_active = collect_active_jobs(active_jobs);
    
    if (n_active == 0) return;

    // Find special jobs
    job* last_job = active_jobs[n_active - 1];
    job* last_stopped = NULL;
    job* prev_stopped = NULL;
    job* last_running = NULL;
    job* plus_job = NULL;
    job* minus_job = NULL;

    // Find stopped and running jobs
    find_stopped_jobs(active_jobs, n_active, &last_stopped, &prev_stopped);
    last_running = find_last_running_job(active_jobs, n_active);

    // Determine markers
    determine_job_markers(last_stopped, prev_stopped, last_running, 
                         last_job, &plus_job, &minus_job);

    // Print job information
    for (int i = 0; i < n_active; i++) {
        print_job_info(active_jobs[i], plus_job, minus_job);
    }
}

/**
 * @brief Check if job can be brought to foreground
 * 
 * @param j Job to check
 * @return int 0 if job can be brought to foreground, 1 otherwise
 */
static int check_fg_job_state(job* j) {
    if (!j) {
        printf("fg: no such job\n");
        return 1;
    }
    return 0;
}

/**
 * @brief Continue job execution in foreground
 * 
 * @param j Job to continue
 */
static void continue_job_in_foreground(job* j) {
    j->status = 1;
    j->bg = 0;
    printf("%s\n", j->job_name);
    
    // Give terminal control to job's process group
    if (is_interactive && tcsetpgrp(STDIN_FILENO, j->job_pgid) == -1) {
        perror("fg fail");
    }
    
    // Continue job execution
    kill(-j->job_pgid, SIGCONT);
    
    // Wait for job completion
    wait_and_handle_job_completion(j);
}

/**
 * @brief Bring a background job to foreground
 * 
 * @param arg Command arguments (can be NULL or job index)
 */
void fg(char **arg) {
    if (!bg_jobs) {
        printf("fg: no such job\n");
        return;
    }
    
    job* j = find_job_by_arg(arg[0]);
    
    if (check_fg_job_state(j) != 0) {
        return;
    }
    
    continue_job_in_foreground(j);
}

/**
 * @brief Add a stopped job to background jobs list
 * 
 * @param curr_job Job to add
 * @param stop_notified Pointer to notification flag
 */
static void handle_stopped_job(job* curr_job, int* stop_notified) {
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
    
    if (!*stop_notified) {
        *stop_notified = 1;
        printf("\n[%d]+  Stopped                 %s\n", curr_job->idx, curr_job->job_name);
    }
}

/**
 * @brief Restore terminal settings for shell
 */
static void restore_terminal_settings() {
    if (is_interactive && tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error setting terminal to shell");
        exit(1);
    }
    if (is_interactive && tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_modes) == -1) {
        perror("tcsetattr error");
        exit(1);
    }
}

/**
 * @brief Wait for job completion and handle its state
 * 
 * @param curr_job Job to wait for
 */
void wait_and_handle_job_completion(job* curr_job) {
    if (!curr_job) {
        return;
    }
    int flag = 0;
    int stop_notified = 0;
    
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
            // Add newline if process was terminated by SIGINT
            if (WTERMSIG(wstatus) == SIGINT) {
                write(STDOUT_FILENO, "\n", 1);
            }
        }
        if (WIFSTOPPED(wstatus)) {
            handle_stopped_job(curr_job, &stop_notified);
        }
    }
    
    if (flag) {
        if (curr_job->idx == biggest_idx && curr_job->idx != 0) {
            biggest_idx--;
        }
        remove_job(curr_job);
        free_job(curr_job);
    }
    
    restore_terminal_settings();
}

/**
 * @brief Handle state change of a job process
 * 
 * @param j Job to handle
 * @param wstatus Process status
 */
static void handle_job_state_change(job* j, int wstatus) {
    if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)) {
        if (j->idx > 0) {  // Don't print Done for simple commands
            printf("[%d]   Done                    %s\n", j->idx, j->job_name);
        }
        if (j->idx != 0 && j->idx == biggest_idx) {
            biggest_idx--;
        }
        j->status = 0;
        remove_job(j);
        free_job(j);
        bg_jobs_number--;
    } else if (WIFSTOPPED(wstatus)) {
        j->status = 2;
        if (!j->bg) {  // Only for foreground jobs
            printf("[%d]+  Stopped                 %s\n", j->idx, j->job_name);
        }
    } else if (WIFCONTINUED(wstatus)) {
        j->status = 1;
    }
}

/**
 * @brief Monitor background jobs and handle their state changes
 */
void monitor_jobs() {
    if (!bg_jobs) {
        biggest_idx = 0;
        return;
    }

    job* j;
    pid_t process_pid;
    int wstatus;

    while ((process_pid = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED | WNOHANG))) {
        if (process_pid == -1) 
            return;
            
        j = find_job(process_pid);
        if (j) {
            handle_job_state_change(j, wstatus);
        }
    }
}

/**
 * @brief Clean up process and its arguments
 * 
 * @param p Process to clean up
 */
void cleanup_process_and_arguments(process* p) {
    if (!p)
        return;
    int i = 0;
    while (p->argv.cmdargs[i]) {
        free(p->argv.cmdargs[i]);
        i++;
    }
}

/**
 * @brief Free a job structure
 * 
 * @param j Job to free
 */
void free_job(job* j) {
    if (!j)
        return;
    process* p = j->processes;
    process* next;
    while (p) {
        next = p->next;
        cleanup_process_and_arguments(p);
        free(p);
        p = next;
    }
    free(j->job_name);
    free(j);
}

/**
 * @brief Find a job by process ID
 * 
 * @param pid Process ID to find
 * @return job* Found job or NULL
 */
job* find_job(pid_t pid) { 
    if (!bg_jobs)
        return NULL;
    job* j = bg_jobs;

    while (j) {
        for (process* p = j->processes; p; p = p->next) {
            if (p->p_pid == pid) {
                return j;
            }
        }
        j = j->next;
    }
    return NULL;
}

/**
 * @brief Remove a job from jobs list
 * 
 * @param j Job to remove
 */
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