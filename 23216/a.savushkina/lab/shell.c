#include <sys/types.h>
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

job_t *jobs = NULL;
job_t *done_bg_jobs = NULL;
string_node_t *new_done_jobs = NULL;
int next_job_number = 1;

int add_job(pid_t pgid, pid_t pid, int fg, int ncmds, struct command cmds[]);
void remove_job(pid_t pgid);
void update_job(pid_t pgid, int status);
job_t *get_job_by_pgid(pid_t pgid);
job_t *get_job_by_pid(pid_t pid);
job_t *get_job_by_job_number(int job_number);
void list_jobs();
void display_done_bg_jobs();
void clear_done_bg_jobs();
void bg_command(int job_number, struct sigaction action);
void fg_command(int job_number, struct sigaction action);
job_t *get_foreground_job();
job_t *get_background_job_pid(pid_t pid);
char *build_command_string(int ncmds, struct command cmds[]);

void sigchld_handler()
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        job_t *job = get_job_by_pid(pid);
        if (job != NULL)
        {
            if (job->fg == BACKGROUND)
            {
                char text[256];
                sprintf(text, "Background job [%d] (%d) completed\n", job->job_number, pid);
                string_node_t *node = malloc(sizeof(string_node_t));
                node->str = strdup(text);
                node->next = new_done_jobs;
                new_done_jobs = node;
                job->next = done_bg_jobs;
                done_bg_jobs = job;
                job->status = -1;
            }
            else
            {
                job->status = -1;
                // remove_job(job->pgid);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    pid_t shell_pid = getpid();
    if (setpgid(shell_pid, shell_pid) < 0)
    {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
    {
        perror("sigaction");
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
            string_node_t *current = new_done_jobs;
            while (current != NULL)
            {
                printf("%s", current->str);
                string_node_t *next = current->next;
                free(current->str);
                free(current);
                current = next;
            }
            new_done_jobs = NULL;
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

        for (int i = 0; i < ncmds; i++)
        {
            if (cmds[i].cmdargs[0] != NULL && strcmp(cmds[i].cmdargs[0], "exit") == 0)
            {
                job_t *job;
                for (job = jobs; job != NULL; job = job->next)
                {
                    kill(-job->pgid, SIGTERM);
                }
                job_t *current, *next;
                for (current = jobs; current != NULL; current = next)
                {
                    next = current->next;
                    free(current);
                }
                jobs = NULL;
                // clear_done_bg_jobs();
                exit(EXIT_SUCCESS);
            }
            if (strcmp(cmds[0].cmdargs[0], "fg") == 0)
            {
                int job_number = atoi(cmds[0].cmdargs[1]);
                fg_command(job_number, blocked_signals);
                continue;
            }
            if (strcmp(cmds[0].cmdargs[0], "bg") == 0)
            {
                int job_number = atoi(cmds[0].cmdargs[1]);
                bg_command(job_number, blocked_signals);
                continue;
            }
            if (strcmp(cmds[0].cmdargs[0], "jobs") == 0)
            {
                if (cmds[0].cmdargs[1] != NULL && strcmp(cmds[0].cmdargs[1], "-l") == 0)
                {
                    display_done_bg_jobs();
                    continue;
                }
                else
                {
                    list_jobs();
                }
                continue;
            }

            pids[i] = fork();
            if (pids[i] < 0)
            {
                perror("fork");
                exit(1);
            }
            else if (pids[i] == 0)
            {
                if (i == 0 || cmds[i].cmdflag == 0)
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
                    if (tcsetpgrp(STDIN_FILENO, (pgid != -1 && cmds[i].cmdflag != 0)  ? pgid : getpid()) < 0)
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
            else
            {
                if (pgid == -1 || cmds[i].cmdflag == 0)
                {
                    pgid = pids[i];
                }
                if (i > 0)
                {
                    close(pipes[i - 1][0]);
                    close(pipes[i - 1][1]);
                }
                int num;
                if (cmds[0].bgk == FOREGROUND)
                {
                    num =  add_job(pgid, pids[i], FOREGROUND, ncmds, cmds);
                }
                else
                {
                    num = add_job(pgid, pids[i], BACKGROUND, ncmds, cmds);
                    job_t *job = get_job_by_job_number(num);
                    printf("[%d] %d\n", job->job_number, job->pid);
                }
                if (cmds[i].cmdflag == 0 && cmds[i].bgk == FOREGROUND)
                {
                    int status;
                    job_t *job = get_job_by_job_number(num);
                    pid_t wpid;
                    // printf("[%d] %d\n", job->job_number, job->pid);
                    if ( wpid = waitpid(pids[i], &status, WUNTRACED) < 0){
                        perror("waitpid in for");
                    }
                    if (WIFEXITED(status) && job->status != 1)
                    {
                        update_job(job->pid, -1);
                        remove_job(job->pid);
                        // printf("Job [%d] exited with status %d\n", fg_job->job_number, WEXITSTATUS(status));
                    }
                    else if (WIFSTOPPED(status))
                    {
                        char *str_signal = strsignal(WSTOPSIG(status));
                        fprintf(stderr, "\tJob [%d] stopped by signal %s (%d)\n", job->job_number, str_signal, WSTOPSIG(status));
                        update_job(job->pid, 1);
                        job->fg = BACKGROUND;
                    }
                    else{
                        update_job(job->pid, -1);
                    }
                }
            }
        }

        for (int i = 0; i < ncmds - 1; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        job_t *fg_job = get_foreground_job();
        if (fg_job != NULL)
        {
            if (fg_job->status != -1){
            int status;
            pid_t wpid;

            do
            {
                pid_t wpid = waitpid(fg_job->pid, &status, WUNTRACED);

                if (wpid == -1)
                {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }

                if (WIFEXITED(status) && fg_job->status != 1)
                {
                    fg_job->status = -1;
                    // printf("Job [%d] exited with status %d\n", fg_job->job_number, WEXITSTATUS(status));
                }
                else if (WIFSTOPPED(status))
                {
                    char *str_signal = strsignal(WSTOPSIG(status));
                    fprintf(stderr, "\tJob [%d] stopped by signal %s (%d)\n", fg_job->job_number, str_signal, WSTOPSIG(status));
                    update_job(fg_job->pgid, 1);
                    fg_job->fg = BACKGROUND;
                }
            } while (wpid != -1 && wpid != 0 && fg_job->fg == FOREGROUND);
            if (fg_job->status != 1)
                remove_job(fg_job->pgid);
            // next_job_number--;
            }
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
        blocked_signals.sa_handler = SIG_IGN;
        sigemptyset(&blocked_signals.sa_mask);
        blocked_signals.sa_flags = 0;
        sigaction(SIGINT, &blocked_signals, NULL);
        sigaction(SIGTSTP, &blocked_signals, NULL);
        sigaction(SIGQUIT, &blocked_signals, NULL);

    }
}

int add_job(pid_t pgid, pid_t pid, int fg, int ncmds, struct command cmds[])
{
    job_t *new_job = malloc(sizeof(job_t));
    new_job->pgid = pgid;
    new_job->pid = pid;
    new_job->job_number = next_job_number++;
    new_job->status = 0;
    new_job->fg = fg;
    new_job->commands = build_command_string(ncmds, cmds);
    new_job->next = jobs;
    jobs = new_job;
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

void remove_job(pid_t pgid)
{
    job_t *job = jobs;
    job_t *prev = NULL;
    while (job != NULL)
    {
        if (job->pgid == pgid)
        {
            if (job->fg == BACKGROUND)
            {
                job->next = done_bg_jobs;
                done_bg_jobs = job;
            }
            else
            {
                if (prev == NULL)
                    jobs = job->next;
                else
                    prev->next = job->next;
                free(job->commands);
                free(job);
            }
            return;
        }
        prev = job;
        job = job->next;
    }
}

void update_job(pid_t pgid, int status)
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        if (job->pgid == pgid)
        {
            job->status = status;
            return;
        }
    }
}

job_t *get_job_by_pgid(pid_t pgid)
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        if (job->pgid == pgid)
            return job;
    }
    return NULL;
}

job_t *get_job_by_pid(pid_t pid)
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        if (job->pid == pid)
            return job;
    }
    return NULL;
}

job_t *get_job_by_job_number(int job_number)
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        if (job->job_number == job_number)
            return job;
    }
    return NULL;
}

void list_jobs()
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        printf("Job: number=%d, fg=%d, pgid=%d, commands=%s\n", job->job_number, job->fg, job->pgid, job->commands ? job->commands : "No command");
    }
    return;
}

void display_done_bg_jobs()
{
    job_t *job;
    if (done_bg_jobs == NULL)
    {
        printf("No completed background jobs.\n");
        return;
    }
    else
    {
        printf("Completed background jobs:\n");
        for (job = done_bg_jobs; job != NULL; job = job->next)
        {
            printf("[%d] (%d) %s\n", job->job_number, job->pid, job->commands);
        }
        return;
    }
}

void clear_done_bg_jobs()
{
    job_t *job, *next;
    for (job = done_bg_jobs; job != NULL; job = next)
    {
        next = job->next;
        free(job->commands);
        free(job);
    }
    done_bg_jobs = NULL;
}

job_t *get_foreground_job()
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        if (job->fg == FOREGROUND)
            return job;
    }
    return NULL;
}

job_t *get_background_job_pid(pid_t pid)
{
    job_t *job;
    for (job = jobs; job != NULL; job = job->next)
    {
        if (job->fg == BACKGROUND && job->pid == pid)
            return job;
    }
    return NULL;
}
void fg_command(int job_number, struct sigaction action)
{
    job_t *job = get_job_by_job_number(job_number);
    if (job == NULL)
    {
        printf("No such job\n");
        return;
    }
    if (job->status == -1){
        printf("Job is already completed\n");
        return;
    }
    if (job->fg == FOREGROUND)
    {
        printf("Job is already in foreground\n");
        return;
    }
    if (job->status == 1)
    {
        printf("Continuing job [%d]\n", job_number);
        kill(-job->pgid, SIGCONT);
    }

    if (tcsetpgrp(STDIN_FILENO, job->pgid) < 0)
    {
        perror("tcsetpgrp");
        exit(EXIT_FAILURE);
    }
    if (tcsetpgrp(STDOUT_FILENO, job->pgid) < 0)
    {
        perror("tcsetpgrp");
        exit(EXIT_FAILURE);
    }
    if (tcsetpgrp(STDERR_FILENO, job->pgid) < 0)
    {
        perror("tcsetpgrp");
        exit(EXIT_FAILURE);
    }

    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    printf("Job [%d] (%d) moved to foreground\n", job_number, job->pid);
    job->fg = FOREGROUND;
    update_job(job->pgid, 0);
}

void bg_command(int job_number, struct sigaction action)
{
    job_t *job = get_job_by_job_number(job_number);
    if (job == NULL)
    {
        printf("No such job\n");
        return;
    }
    if (job->status == -1){
        printf("Job is already completed\n");
        return;
    }
    if (job->status == 1)
    {
        printf("Continuing job [%d] (%d)\n", job_number, job->pid);
        kill(-job->pgid, SIGCONT);
    }

    if (job->fg != BACKGROUND)
        printf("Job [%d] (%d) moved to background\n", job_number, job->pid);
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    job->fg = BACKGROUND;
    update_job(job->pgid, 0);
}