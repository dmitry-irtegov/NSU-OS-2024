#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include "shell.h"

#define BACKGROUND 0
#define FOREGROUND 1

struct command cmds[MAXCMDS];

/* A process is a single process.  */
typedef struct process
{
    struct process *next;
    char **argv;
    pid_t pid;
    char completed;
    char stopped;
    int status;
} process;

/* A job is a pipeline of processes.  */
typedef struct job
{
    struct job *next;          /* next active job */
    int job_number;            /* unique job number */
    char *command;             /* command line, used for messages */
    process *first_process;    /* list of processes in this job */
    pid_t pgid;                /* process group ID */
    char notified;             /* true if user told about stopped job */
    struct termios tmodes;     /* saved terminal modes */
    // int stdin, stdout, stderr; /* standard i/o channels */
} job;

typedef struct string_node
{
    char *str;
    struct string_node *next;
} string_node_t;

job *first_job = NULL;
job *done_bg_jobs = NULL;
string_node_t *new_done_jobs = NULL;
int next_job_number = 1;
struct sigaction blocked_signals;

int add_job(pid_t pgid, pid_t *pids, int nprocs, int fg, int ncmds, struct command cmds[]);
void remove_job(pid_t pgid);
void update_job(pid_t pgid, int status);
job *get_job_by_pgid(pid_t pgid);
job *get_job_by_pid(pid_t pid);
job *get_job_by_job_number(int job_number);
void list_jobs();
void display_done_bg_jobs();
void clear_done_bg_jobs();
void bg_command(int job_number);
void fg_command(int job_number);
job *get_foreground_job();
job *get_background_job();
char *build_command_string(int ncmds, struct command cmds[]);

int main(int argc, char *argv[])
{
    pid_t shell_pid = getpid();
    if (setpgid(shell_pid, shell_pid) < 0)
    {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }

    struct sigaction blocked_signals;
    blocked_signals.sa_handler = SIG_IGN;
    sigemptyset(&blocked_signals.sa_mask);
    sigaction(SIGINT, &blocked_signals, NULL);
    sigaction(SIGTSTP, &blocked_signals, NULL);
    sigaction(SIGQUIT, &blocked_signals, NULL);

    struct sigaction ignore;
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    sigaction(SIGTTOU, &ignore, NULL);
    sigaction(SIGTTIN, &ignore, NULL);

    char line[1024];
    int ncmds;
    char prompt[50];
    sprintf(prompt, "[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0)
    {
        if ((ncmds = parseline(line)) <= 0)
        {
            // do_job_notification();
            continue;
        }

        int pipes[ncmds - 1][2];
        for (int i = 0; i < ncmds - 1; i++)
        {
            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
                exit(1);
            }
        }

        pid_t pids[ncmds];
        pid_t pgid = -1;

        if (cmds[0].cmdargs[0] != NULL && strcmp(cmds[0].cmdargs[0], "exit") == 0)
        {
                struct job *job;
                for (job = first_job; job != NULL; job = job->next)
                {
                    kill(-job->pgid, SIGTERM);
                }
                struct job *current, *next;
                for (current = first_job; current != NULL; current = next)
                {
                    next = current->next;
                    free(current->command);
                    free(current);
                }
                first_job = NULL;
                exit(EXIT_SUCCESS);
        }

        {
            for (int i = 0; i < ncmds; i++)
            {

                pids[i] = fork();
                if (pids[i] < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pids[i] == 0)
                {
                    if (i == 0)
                    {
                        if (setpgid(0, 0) < 0)
                        {
                            perror("setpgid");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        if (setpgid(0, pgid) < 0)
                        {
                            perror("setpgid");
                            exit(EXIT_FAILURE);
                        }
                    }

                    if (cmds[0].bgk == FOREGROUND)
                    {
                        if (tcsetpgrp(STDIN_FILENO, (pgid != -1 && cmds[i].cmdflag != 0) ? pgid : getpid()) < 0)
                        {
                            perror("tcsetpgrp");
                            exit(EXIT_FAILURE);
                        }
                        if (tcsetpgrp(STDOUT_FILENO, (pgid != -1 && cmds[i].cmdflag != 0) ? pgid : getpid()) < 0)
                        {
                            perror("tcsetpgrp");
                            exit(EXIT_FAILURE);
                        }
                        if (tcsetpgrp(STDERR_FILENO, (pgid != -1 && cmds[i].cmdflag != 0) ? pgid : getpid()) < 0)
                        {
                            perror("tcsetpgrp");
                            exit(EXIT_FAILURE);
                        }
                    }

                    blocked_signals.sa_handler = SIG_DFL;
                    sigemptyset(&blocked_signals.sa_mask);
                    blocked_signals.sa_flags = 0;
                    sigaction(SIGINT, &blocked_signals, NULL);
                    sigaction(SIGTSTP, &blocked_signals, NULL);
                    sigaction(SIGQUIT, &blocked_signals, NULL);

                    if (strcmp(cmds[0].cmdargs[0], "jobs") == 0)
                    {
                        // if (cmds[0].cmdargs[1] != NULL)
                        // {
                        //     if (strcmp(cmds[0].cmdargs[1], "-l") == 0)
                        //         {
                        //             do_job_notification();
                        //         }
                        //         else
                        //         {
                        //             fprintf(stderr, "Invalid option for jobs command\n");
                        //         }
                        // }
                        // else{
                        list_jobs();
                        // }
                        exit(EXIT_SUCCESS);
                    }
                    if (strcmp(cmds[0].cmdargs[0], "fg") == 0)
                    {
                        int job_number = atoi(cmds[0].cmdargs[1]);
                        fg_command(job_number);
                    }
                    else if (strcmp(cmds[0].cmdargs[0], "bg") == 0)
                    {
                        int job_number = atoi(cmds[0].cmdargs[1]);
                        bg_command(job_number);
                    }
                    else{

                    if (cmds[i].bgk == BACKGROUND && cmds[i].infile == NULL)
                    {
                        int null_fd = open("/dev/null", O_RDONLY);
                        if (null_fd != -1)
                        {
                            dup2(null_fd, 0);
                            close(null_fd);
                        }
                    }
                    if (cmds[i].infile != NULL)
                    {
                        int infile_fd = open(cmds[i].infile, O_RDONLY);
                        if (infile_fd < 0)
                        {
                            perror("open infile failed");
                            exit(EXIT_FAILURE);
                        }
                        dup2(infile_fd, 0);
                        close(infile_fd);
                    }

                    if (cmds[i].outfile != NULL)
                    {
                        int outfile_fd = open(cmds[i].outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (outfile_fd < 0)
                        {
                            perror("open outfile failed");
                            exit(EXIT_FAILURE);
                        }
                        dup2(outfile_fd, 1);
                        close(outfile_fd);
                    }
                    else if (cmds[i].appfile != NULL)
                    {
                        int appfile_fd = open(cmds[i].appfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if (appfile_fd < 0)
                        {
                            perror("open appfile failed");
                            exit(EXIT_FAILURE);
                        }
                        dup2(appfile_fd, 1);
                        close(appfile_fd);
                    }
                    if (cmds[i].cmdflag > 0)
                    {
                        if (i > 0)
                        {
                            dup2(pipes[i - 1][0], 0);
                            close(pipes[i - 1][1]);
                        }
                        if (i < ncmds - 1)
                        {
                            dup2(pipes[i][1], 1);
                            close(pipes[i][0]);
                        }
                    }

                    execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    if (pgid == -1)
                    {
                        pgid = pids[i];
                    }
                    if (i > 0)
                    {
                        close(pipes[i - 1][0]);
                        close(pipes[i - 1][1]);
                    }
                    int num;
                }
            }
        }
        if (cmds[0].bgk == FOREGROUND && !(strcmp(cmds[0].cmdargs[0], "bg") == 0))
        {
            int job_number;
            if (strcmp(cmds[0].cmdargs[0], "fg") == 0)
            {
                job_number = atoi(cmds[0].cmdargs[1]);
                job* job_fg = get_job_by_job_number(job_number);
                if (job_fg){
                    pgid = job_fg->pgid;
                }
            }
            else
                job_number = add_job(pgid, pids, ncmds, FOREGROUND, ncmds, cmds);
            // Handle foreground job waiting
            int status;
            pid_t wpid;
            do
            {
                wpid = waitpid(-pgid, &status, WUNTRACED);
                // printf("%d\n", wpid);
                if (wpid == -1 && errno != ECHILD)
                {
                    perror("waitpid");
                    break;
                }
                if (wpid > 0)
                {
                    job *job_ptr = get_job_by_pid(wpid);
                    if (job_ptr)
                    {
                        process *proc_ptr = job_ptr->first_process;
                        while (proc_ptr)
                        {
                            if (proc_ptr->pid == wpid)
                            {
                                printf("\nlol %d  %d\n", proc_ptr->pid, wpid);
                                // if (WI)
                                if (WIFEXITED(status))
                                {
                                    proc_ptr->completed = 1;
                                    proc_ptr->status = WEXITSTATUS(status);
                                    printf("exit status = %d\n", proc_ptr->status);
                                }
                                else if (WIFSIGNALED(status))
                                {
                                    proc_ptr->completed = 1;
                                    proc_ptr->status = WTERMSIG(status);
                                    printf("terminated by signal %d\n", proc_ptr->status);
                                }
                                else if (WIFSTOPPED(status))
                                {
                                    update_job_stop(job_ptr->pgid);
                                    update_job(job_ptr->pgid, WSTOPSIG(status));
                                    printf(" [%d] stopped by signal %d\n", job_number, proc_ptr->status);
                                }
                                break;
                            }
                            proc_ptr = proc_ptr->next;
                        }
                    }
                }
            } while (!job_completed(first_job));
        }
        else
        {
            int job_number;
            if (!(strcmp(cmds[0].cmdargs[0], "bg") == 0) && !(strcmp(cmds[0].cmdargs[0], "fg") == 0)){
                job_number = add_job(pgid, pids, ncmds, BACKGROUND, ncmds, cmds);
            }
            else{
                job_number = atoi(cmds[0].cmdargs[1]);
                job* job_fg = get_job_by_job_number(job_number);
                if (job_fg){
                    pgid = job_fg->pgid;
                }
            }
            printf("[%d] %d\n", job_number, pgid);
        }

        if (job_completed(first_job) && !job_is_stopped(first_job))
        {
            remove_job(first_job->pgid);
        }

        // Clean up pipes and signals
        for (int i = 0; i < ncmds - 1; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        if (tcsetpgrp(STDIN_FILENO, shell_pid) < 0)
        {
            perror("tcsetpgrp stdin");
            exit(EXIT_FAILURE);
        }
        if (tcsetpgrp(STDERR_FILENO, shell_pid) < 0)
        {
            perror("tcsetpgrp stderr");
            exit(EXIT_FAILURE);
        }
        if (tcsetpgrp(STDOUT_FILENO, shell_pid) < 0)
        {
            perror("tcsetpgrp stdout");
            exit(EXIT_FAILURE);
        }

        // setpgid(0, shell_pid);
        do_job_notification();
    }
}

void do_job_notification(void)
{
    job *j, *jlast, *jnext;
    update_status();
    jlast = NULL;
    for (j = first_job; j; j = jnext)
    {
        jnext = j->next;
        if (job_is_completed(j))
        {
            format_job_info(j, "completed");
            if (jlast)
                jlast->next = jnext;
            else
                first_job = jnext;
            free_job(j);
        }
        else if (job_is_stopped(j) && !j->notified)
        {
            format_job_info(j, "stopped");
            j->notified = 1;
            jlast = j;
        }
        else
        {
            jlast = j;
        }
    }
}

void free_job(struct job *job)
{
    free_processes(job->first_process);
    free(job);
}

void free_processes(struct process *process)
{
    struct process *temp;
    while (process != NULL)
    {
        temp = process;
        process = process->next;
        free(temp);
    }
}

int mark_process_status(pid_t pid, int status)
{
    job *j;
    process *p;

    if (pid > 0)
    {
        /* Update the record for the process.  */
        for (j = first_job; j; j = j->next)
            for (p = j->first_process; p; p = p->next)
                if (p->pid == pid)
                {
                    p->status = status;
                    p->completed = 1;
                    return 0;
                }
        fprintf(stderr, "No child process %d.\n", pid);
        return -1;
    }
    else if (pid == 0 || errno == ECHILD)
        /* No processes ready to report.  */
        return -1;
    else
    {
        /* Other weird errors.  */
        perror("waitpid");
        return -1;
    }
}

void format_job_info(job *j, const char *status)
{
    fprintf(stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

void update_status(void)
{
    int status;
    pid_t pid;

    do
    {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid == -1 && errno != ECHILD)
        {
            perror("waitpid");
            break;
        }
    } while (!mark_process_status(pid, status));
}

int add_job(pid_t pgid, pid_t *pids, int nprocs, int fg, int ncmds, struct command cmds[])
{
    job *new_job = malloc(sizeof(job));
    if (!new_job)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_job->pgid = pgid;
    new_job->job_number = next_job_number++;
    new_job->command = build_command_string(ncmds, cmds);
    new_job->notified = 0;
    new_job->first_process = NULL;
    new_job->next = first_job;
    first_job = new_job;

    for (int i = 0; i < nprocs; i++)
    {
        process *new_proc = malloc(sizeof(process));
        if (!new_proc)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        new_proc->argv = cmds[i].cmdargs;
        new_proc->pid = pids[i];
        new_proc->completed = 0;
        new_proc->stopped = 0;
        new_proc->status = 0;
        new_proc->next = new_job->first_process;
        new_job->first_process = new_proc;
    }
    return new_job->job_number;
}

char *build_command_string(int ncmds, struct command cmds[])
{
    int total_length = 1;
    for (int i = 0; i < ncmds; i++)
    {
        total_length += strlen(cmds[i].cmdargs[0]) + (i < ncmds - 1 ? 3 : 0);
    }
    char *commands = malloc(total_length);
    if (commands == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *commands = '\0';
    for (int i = 0; i < ncmds; i++)
    {
        strcat(commands, cmds[i].cmdargs[0]);
        if (i != ncmds - 1)
            strcat(commands, " | ");
    }
    return commands;
}

void remove_job(pid_t pid)
{
    job *job_ptr = first_job;
    job *prev_job = NULL;
    while (job_ptr)
    {
        process *proc_ptr = job_ptr->first_process;
        process *prev_proc = NULL;
        while (proc_ptr)
        {
            if (proc_ptr->pid == pid)
            {
                if (prev_proc)
                {
                    prev_proc->next = proc_ptr->next;
                }
                else
                {
                    job_ptr->first_process = proc_ptr->next;
                }
                free(proc_ptr);
                break;
            }
            prev_proc = proc_ptr;
            proc_ptr = proc_ptr->next;
        }
        if (job_ptr->first_process == NULL)
        {
            // Job has no more processes, remove the job
            if (prev_job)
            {
                prev_job->next = job_ptr->next;
            }
            else
            {
                first_job = job_ptr->next;
            }
            free(job_ptr);
            break;
        }
        prev_job = job_ptr;
        job_ptr = job_ptr->next;
    }
}

void update_job(pid_t pgid, int status)
{
    job *job_ptr = first_job;
    while (job_ptr)
    {
        if (job_ptr->pgid == pgid)
        {
            process *proc_ptr = job_ptr->first_process;
            while (proc_ptr)
            {
                if (proc_ptr->pid == pgid)
                {
                    proc_ptr->status = status;
                    break;
                }
                proc_ptr = proc_ptr->next;
            }
            break;
        }
        job_ptr = job_ptr->next;
    }
}
void update_job_start(pid_t pid)
{
    job *job_ptr = first_job;
    while (job_ptr)
    {
        if (job_ptr->pgid == pid)
        {
            process *proc_ptr = job_ptr->first_process;
            while (proc_ptr)
            {
                if (proc_ptr->pid == pid)
                {
                    proc_ptr->stopped = 0;
                    break;
                }
                proc_ptr = proc_ptr->next;
            }
            break;
        }
        job_ptr = job_ptr->next;
    }
}
void update_job_stop(pid_t pgid)
{
    job *job_ptr = first_job;
    while (job_ptr)
    {
        if (job_ptr->pgid == pgid)
        {
            process *proc_ptr = job_ptr->first_process;
            while (proc_ptr)
            {
                if (proc_ptr->pid == pgid)
                {
                    proc_ptr->stopped = 1;
                    break;
                }
                proc_ptr = proc_ptr->next;
            }
            break;
        }
        job_ptr = job_ptr->next;
    }
}

job *get_job_by_pgid(pid_t pgid)
{
    job *job;
    for (job = first_job; job != NULL; job = job->next)
    {
        if (job->pgid == pgid)
            return job;
    }
    return NULL;
}

job *get_job_by_pid(pid_t pid)
{
    job *job_ptr = first_job;
    while (job_ptr)
    {
        process *proc_ptr = job_ptr->first_process;
        while (proc_ptr)
        {
            if (proc_ptr->pid == pid)
            {
                return job_ptr;
            }
            proc_ptr = proc_ptr->next;
        }
        job_ptr = job_ptr->next;
    }
    return NULL;
}

job *get_job_by_job_number(int job_number)
{
    job *current = first_job;
    while (current != NULL)
    {
        if (current->job_number == job_number)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void list_jobs()
{
    job *job_ptr = first_job;
    if (job_ptr == NULL)
    {
        printf("No jobs.\n");
        return;
    }
    while (job_ptr != NULL)
    {
        printf("Job JOB_NUMBER: %d PGID: %d, Command: %s\n", job_ptr->job_number, job_ptr->pgid, job_ptr->command);
        process *proc_ptr = job_ptr->first_process;
        while (proc_ptr)
        {
            printf("\tProcess PID: %d, Status: %d\n", proc_ptr->pid, proc_ptr->status);
            proc_ptr = proc_ptr->next;
        }
        job_ptr = job_ptr->next;
    }
}

/* Find the active job with the indicated pgid.  */
job *find_job(pid_t pgid)
{
    job *j;

    for (j = first_job; j; j = j->next)
        if (j->pgid == pgid)
            return j;
    return NULL;
}

int job_completed(job *job_ptr)
{
    process *proc_ptr = job_ptr->first_process;
    while (proc_ptr)
    {
        if (!proc_ptr->completed && !proc_ptr->stopped)
        {
            return 0;
        }
        proc_ptr = proc_ptr->next;
    }
    return 1;
}
/* Return true if all processes in the job have stopped or completed.  */
int job_is_stopped(job *j)
{
    process *p;

    for (p = j->first_process; p; p = p->next)
        if (!p->stopped)
            return 0;
    return 1;
}

/* Return true if all processes in the job have completed.  */
int job_is_completed(job *j)
{
    process *p;

    for (p = j->first_process; p; p = p->next)
        if (!p->completed)
            return 0;
    return 1;
}

void fg_command(int job_number)
{
    job *job_ptr = get_job_by_job_number(job_number);
    if (job_ptr == NULL)
    {
        printf("No such job\n");
        return;
    }
    printf("Continuing job [%d]\n", job_number);
    // if (setpgid(0, job_ptr->pgid) < 0) {
    //     perror("setpgid");
    //     return; // Avoid exiting the shell
    // }
    kill(-job_ptr->pgid, SIGCONT);

    if (tcsetpgrp(STDIN_FILENO, job_ptr->pgid) < 0 ||
        tcsetpgrp(STDOUT_FILENO, job_ptr->pgid) < 0 ||
        tcsetpgrp(STDERR_FILENO, job_ptr->pgid) < 0)
    {
        perror("tcsetpgrp");
        exit(EXIT_FAILURE); // Avoid exiting the shell
    }
    update_job_start(job_ptr->pgid);
    update_job(job_ptr->pgid, 0);
    printf("Job [%d] (%d) moved to foreground\n", job_number, job_ptr->pgid);

    // // Wait for the job to finish
    // int status;
    // pid_t wpid;
    // do {
    //     wpid = waitpid(-job_ptr->pgid, &status, 0);
    //     if (wpid == -1 && errno != ECHILD) {
    //         perror("waitpid");
    //         break;
    //     }
    //     if (wpid > 0) {
    //         // Handle the status
    //         update_job(job_ptr->pgid, status);
    //     }
    // } while (!job_completed(job_ptr));
}
void bg_command(int job_number)
{
    job *job_ptr = get_job_by_job_number(job_number);
    if (job_ptr == NULL)
    {
        printf("No such job\n");
        return;
    }
    // if (job_ptr->status == -1) {
    //     printf("Job is already completed\n");
    //     return;
    // }
    printf("Continuing job [%d] (%d)\n", job_number, job_ptr->pgid);
    // if (setpgid(0, job_ptr->pgid) < 0) {
    //     perror("setpgid");
    //     return;
    // }
    kill(-job_ptr->pgid, SIGCONT);

    // if (job_ptr->fg != BACKGROUND) {
    printf("Job [%d] (%d) moved to background\n", job_number, job_ptr->pgid);
    // }
    // job_ptr->fg = BACKGROUND;
    update_job_start(job_ptr->pgid);
    update_job(job_ptr->pgid, 0);
}