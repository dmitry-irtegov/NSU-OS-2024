#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include "shell.h"

char *infile, *outfile, *appfile;
char prompt[1050]; /* shell prompt */
struct command cmds[MAXCMDS];
char bkgrnd;
pid_t pid;
pid_t group_id;
pid_t shell;

typedef struct process {
    struct process* next; 
    pid_t p_pid; 
    pid_t g_pid; 
    struct command argv;
    int status; 
    int fildes[2];
    int prev_fildes[2];
} process;

typedef struct job {
    struct job* next;
    pid_t job_pgid;
    process* processes;
    int status;
    int fildes[2];
} job;

job* stopped_jobs = NULL;
job* bg_jobs = NULL;
void start_process(process* p);
void start_job(job* job);
void wait_for_job(job* job);
void sigcatch(int sig);
void sigcatch_sigtstp(int sig);
void fg(char* arg);
void bg(char* arg);
int main(int argc, char *argv[])
{
    shell = getpid();
    setpgid(shell, shell);
    tcsetpgrp(STDIN_FILENO, shell);
    char cwd[1025];
    getcwd(cwd, sizeof(cwd));
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    register int i;
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    int fd[2];
    int prev_fd = -1;

    sprintf(prompt,"%s\n$ ", cwd);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /*
until eof  */
        if (tcsetpgrp(STDIN_FILENO, shell) == -1) {
            perror("error setting terminal to shell 2");
        }
        group_id = 0;
        if ((ncmds = parseline(line)) <= 0)
            continue;   /* read next line */
#ifdef DEBUG
{
    int i, j;
    for (i = 0; i < ncmds; i++) {
        for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
            fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
            i, j, cmds[i].cmdargs[j]);
        fprintf(stderr, "cmds[%d].cmdflag = %o\n", i,
cmds[i].cmdflag);
    }
}
#endif
        process* head_p = (process*) malloc(sizeof(process));
        process* p_next;
        job* j = malloc(sizeof(job));
        j->processes = head_p;
        j->job_pgid = 0;
        j->next = NULL;
        process* curr_p = head_p;
        for (i = 0; i < ncmds; i++) {
            curr_p->argv = cmds[i];
            curr_p->prev_fildes[0] = 0;
            curr_p->prev_fildes[1] = 0;
            if (i < ncmds -1) {
                p_next = (process*) malloc(sizeof(process));
                curr_p->next = p_next;
                curr_p = p_next;
            } else {
                curr_p->next = NULL;
            }
        }
        start_job(j);
    }  
    return 0;
}

void start_process(process* p) {
    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_DFL);
    signal (SIGTSTP, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
    signal (SIGCHLD, SIG_DFL);

    if (setpgid(p->p_pid, p->g_pid)== -1) {
        perror("setpgid error child");
    } 
    if (bkgrnd == 0) {
        if (tcsetpgrp(STDIN_FILENO, p->g_pid) == -1) { 
            perror("error setting terminal child");
        } 
    }

    if (infile) { 
        int in_desc = open(infile, O_RDONLY);
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
    if (outfile) { 
        int out_desc = open(outfile, O_WRONLY);
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
    if (appfile) { 
        int app_desc = open(appfile, O_APPEND | O_WRONLY);
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
        if (p->argv.cmdflag != 3) {
            close(p->prev_fildes[1]);
        }
    }
    if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) { 
        if (dup2(p->fildes[1], STDOUT_FILENO) == -1) {
            perror("redirecting stdout pipe error");
            exit(1);
        }
        close(p->fildes[0]);
        close(p->fildes[1]);
    }

    execvp(p->argv.cmdargs[0], p->argv.cmdargs);
    perror("execvp error");
    exit(1);
}

void start_job(job* curr_job) {
    signal(SIGTTOU, SIG_IGN);
    for (process* p = curr_job->processes; p; p = p->next) {
        if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) { 
            if (pipe(p->fildes) == -1) { 
                perror("pipe error");
                exit(1);
            }
            p->next->prev_fildes[0] = p->fildes[0];
            p->next->prev_fildes[1] = p->fildes[1];
        }

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
                if (tcsetpgrp(STDIN_FILENO, curr_job->job_pgid) == -1) { 
                    perror("child getting terminal error");
                } 
            }
            p->g_pid = curr_job->job_pgid;
            if (setpgid(p->p_pid, curr_job->job_pgid) == -1) { 
                perror("setpgid error parent");
            } 
            if (bkgrnd != 0) { 
                fprintf(stderr, "bg job %d\n", pid);
            }
    
            if (p->argv.cmdflag == OUTPIP || p->argv.cmdflag == 3) {
                close (p->fildes[1]);
                p->next->prev_fildes[0] = p->fildes[0];
            }
            if (p->prev_fildes[0] != 0) {
                close(p->prev_fildes[0]);
            }
        }
    }
    if (bkgrnd == 0) { 
       wait_for_job(curr_job);
    }
    if (tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error setting terminal to shell 1");
    }
}

void wait_for_job(job* curr_job) {
    int wstatus;
    for (process* p = curr_job->processes; p; p = p->next) {
        int wstatus;
        if (waitpid(-curr_job->job_pgid, &wstatus, WUNTRACED) == -1) { 
            perror("waitpid error");
            exit(1);
        }
        if (WIFSTOPPED(wstatus)) {
            fprintf(stderr, "%d\n", p->p_pid);
        }
    }
    kill(shell, SIGCONT);
    if (tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error setting terminal to shell");
    }
}
