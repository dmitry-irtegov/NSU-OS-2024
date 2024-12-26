#include "shell.h"
// #define DEBUG

job* job_list = NULL;
int job_counter = 1;

char c;

char *infile, *outfile, *appfile;
struct termios shell_attr;
int shell_is_interactive;
struct command cmds[MAXCMDS];
char bkgrnd;
pid_t shell_pgid;


// void print_process(job *j) {
//     process *p = j->processes;
//     printf("job pgid -  %d, ground = %d\n", j->pgid, j->ground);
// }

int main(int argc, char *argv[])
{
    init_shell();
    char line[1024];      /*  allow large command lines  */
    int ncmds;
    char prompt[50];      /* shell prompt */
    char *cwd = getcwd(NULL, 512);

    sprintf(prompt,"\n[%s] ", cwd);

    free(cwd);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /*until eof  */
        if ((ncmds = parseline(line)) <= 0)
            continue;   /* read next line */
            
        job* new_job = create_job_from_cmds(line, cmds, ncmds);
        
        update_job_statuses();
        start_job(new_job);
        
        // print_process(new_job);
        cwd = getcwd(NULL, 512);
        sprintf(prompt,"\n[%s] ", cwd);
        free(cwd);

#ifdef DEBUG
    {
        int i, j;
            for (i = 0; i < ncmds; i++) {
            for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
                fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n", i, j, cmds[i].cmdargs[j]);
            fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
        }
    }
#endif
    }   

}

void init_shell() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("Couldn't put the shell in its own process group");
        exit(EXIT_FAILURE);
    }

    tcsetpgrp(STDIN_FILENO, shell_pgid);

    tcgetattr(STDIN_FILENO, &shell_attr);
}

void start_job(job* job_c) {
    int status;
    int pipe1_fd[2] = {0};
    int pipe2_fd[2] = {0};

    if (bkgrnd) {
        job_c->ground = 1;
    } 

    add_job(job_c);

    if (strcmp(job_c->processes->argv.cmdargs[0], "cd") == 0) {
        cd(job_c->processes->argv.cmdargs[1]);
        remove_job(job_c);
        return;
    }
    if (strcmp(job_c->processes->argv.cmdargs[0], "fg") == 0) {
        fg(job_c->processes->argv.cmdargs[1]);
        remove_job(job_c);
        return;
    }
    if (strcmp(job_c->processes->argv.cmdargs[0], "bg") == 0) {
        bg(job_c->processes->argv.cmdargs[1]);
        remove_job(job_c);
        return;
    }
    if (strcmp(job_c->processes->argv.cmdargs[0], "jobs") == 0) {
        jobs();
        remove_job(job_c);
        return;
    } 
    if (strcmp(job_c->processes->argv.cmdargs[0], "exit") == 0) {
        job* j = job_list;
        while (j) {
            job* next_job = j->next;  
            remove_job(j);         
            j = next_job;
        }
        exit(EXIT_SUCCESS);
    }

    process *p_c = job_c->processes;
    job_c->status = RUNNING;
    while (p_c) {
        p_c->status = RUNNING;
        p_c = p_c->next;
    }

    for (process* p = job_c->processes; p; p = p->next) {
        pipe2_fd[0] = pipe1_fd[0];
        pipe2_fd[1] = pipe1_fd[1];

        if ((p->argv.cmdflag & OUTPIP)) {
            if (pipe(pipe1_fd) == -1) {
                perror("pipe() failed");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid = fork();
        switch (pid) {
            case -1:
                perror("fork() failed");
                exit(EXIT_FAILURE);
                break;
            case 0: 
                if (p == job_c->processes) {
                    if (setpgid(0, 0) == -1) {
                        perror("setpgid() failed in child");
                    }
                } else {
                    if (setpgid(0, job_c->pgid) == -1) {
                        perror("setpgid() failed in child");
                    }
                }
                
                start_process(p, pipe1_fd, pipe2_fd); 
                break;
            default: 
                p->pid = pid;
                if (p == job_c->processes) {
                    job_c->pgid = pid; 
                }
                if (setpgid(pid, job_c->pgid) != 0) {
                    perror("setpgid() failed in parent");
                }

                if (!bkgrnd) {
                    if (tcsetpgrp(STDIN_FILENO, job_c->pgid) == -1) {
                        perror("tcsetpgrp() failed");
                    }
                }


                if (p->next && (p->argv.cmdflag & OUTPIP)) {
                    close(pipe1_fd[1]); 
                }
                if (p->prev && (p->prev->argv.cmdflag & OUTPIP)) {
                    close(pipe2_fd[0]);
                }
                break;
        }
    }

    if (bkgrnd) {
        printf("%d\n", job_c->processes->pid); 
    } else {
        for (process* p = job_c->processes; p; p = p->next) {
            if (waitpid(p->pid, &status, WUNTRACED) == -1) {
                perror("waitpid() failed");
                exit(EXIT_FAILURE);
            }

            if (WIFSTOPPED(status)) {
                p->status = STOP; 
                job_c->ground = 1;
                printf("\nStopped job %d: %s\n", job_c->number, job_c->command);
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                p->status = DONE; 
            }
        }

        update_job_statuses();
        if (job_c->status == DONE) {
            remove_job(job_c);
        }
        if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1) {
            perror("tcsetpgrp() failed");
        }
    }
}

void start_process(process* p, int pipe1_fd[], int pipe2_fd[]) {
    // tcsetpgrp(STDIN_FILENO, p->pid);
    p->status = RUNNING;
    
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    if (infile) {
        int in = open(infile, O_RDONLY);
        if (in == -1) {
            perror("open() fail");
            exit(EXIT_FAILURE);
        }
        if (dup2(in, STDIN_FILENO) == -1) {
            perror("dup2() fail");
            exit(EXIT_FAILURE);
        }
        close(in);
    }
    
    if (outfile) {
        int out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        if (out == -1) {
            perror("open() fail");
            exit(EXIT_FAILURE);
        }
        if (dup2(out, STDOUT_FILENO) == -1) {
            perror("dup2() fail");
            exit(EXIT_FAILURE);
        }
        close(out);
    }

    if (appfile) {
        int app = open(appfile, O_APPEND | O_WRONLY | O_CREAT, 0664);
        if (app == -1) {
            perror("open() fail");
            exit(EXIT_FAILURE);
        }
        if (dup2(app, STDOUT_FILENO) == -1) {
            perror("dup2() fail");
            exit(EXIT_FAILURE);
        }
        close(app);
    }

    if ((p->argv.cmdflag & INPIP)) {
        
        if (dup2(pipe2_fd[0], STDIN_FILENO) == -1) {
            perror("dup2() inpip failed");
            exit(EXIT_FAILURE);
        }
        close(pipe2_fd[0]); 
        if (p->argv.cmdflag != 3) 
            close(pipe2_fd[1]);
    }

    if ((p->argv.cmdflag & OUTPIP)) {
        close(pipe1_fd[0]); 
        if (dup2(pipe1_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2() outpip failed");
            exit(EXIT_FAILURE);
        }
        close(pipe1_fd[1]); 
    }

    if (execvp(p->argv.cmdargs[0], p->argv.cmdargs) == -1) {
        perror("execvp() failed, command not found");
        exit(EXIT_FAILURE);
    }
}

int update_job_statuses() {
    int status;
    pid_t pid;
    int flag = 1;
    job* curr_job = job_list;
    while (curr_job) {
        int is_stopped = 1;
        int is_running = 0; 
        int is_done = 1;

        process* p = curr_job->processes;
        while (p) {
            if (p->status != DONE) {
                pid = waitpid(p->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
                if (pid == -1) {
                    return -1;
                }

                if (pid > 0) {
                    if (WIFSTOPPED(status)) {
                        p->status = STOP;
                        curr_job->ground = 1;
                    } else if (WIFCONTINUED(status)) {
                        p->status = RUNNING;
                    } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                        p->status = DONE;
                    }
                }
            }

            if (p->status == RUNNING) {
                is_stopped = 0;
                is_running = 1;
                is_done = 0;     
            } else if (p->status == STOP) {
                is_stopped = 1;  
                is_running = 0;  
                is_done = 0;
            } else if (p->status == DONE) {
                is_stopped = 0;
                is_running = 0;
                // is_done = 1;
            }

            p = p->next;
        }
        

        if (is_done) {
            curr_job->status = DONE;
        } else if (is_stopped) {
            curr_job->status = STOP;
        } else if (is_running) {
            curr_job->status = RUNNING;
        } else {
            flag = 0;
        }

        curr_job = curr_job->next;
    }
    return flag;
}

job* create_job_from_cmds(char* command, struct command cmds[], int ncmds) {
    job* new_job = malloc(sizeof(job));
    if (!new_job) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    new_job->number = job_counter++;
    new_job->command = strdup(command);
    new_job->processes = NULL;
    new_job->pgid = 0;
    new_job->status = DONE; // 
    new_job->p_count = ncmds;
    new_job->next = NULL;
    new_job->prev = NULL;
    new_job->ground = 0;

    process* prev_process = NULL;
    for (int i = 0; i < ncmds; i++) {
        process* new_process = malloc(sizeof(process));
        if (!new_process) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        new_process->argv = cmds[i];
        new_process->pid = 0;
        new_process->status = 0;
        new_process->next = NULL;
        new_process->prev = prev_process;

        if (prev_process) {
            prev_process->next = new_process;
        } else {
            new_job->processes = new_process;
        }

        prev_process = new_process;
    }

    return new_job;
}

job* add_job(job* job_c) {
    if (!job_list) {
        job_list = job_c;
    } else {
        job* last = job_list;
        while (last->next) last = last->next;
        last->next = job_c;
        job_c->prev = last;
    }
    
    return job_c;
}

void remove_job(job* j) {
    if (!j) return;

    if (j->prev) {
        j->prev->next = j->next;
    } else {
        job_list = j->next; 
    }

    if (j->next) {
        j->next->prev = j->prev;
    }

    process* p = j->processes;
    while (p) {
        process* next = p->next;
        free(p);
        p = next;
    }

    free(j->command); 
    free(j);

    if (job_list == NULL) {
        job_counter = 1;
    }
}

void cd(char* arg) {
    const char *home;
    if (arg == NULL) {
        home = getenv("HOME");
        if (home != NULL) {
            if (chdir(home) != 0) {
                perror("cd to HOME fail");
            }
        } else {
            fprintf(stderr, "cd: HOME not set\n");
        }
    } else if (chdir(arg) != 0) {
        perror("cd failed");
    }
}

void fg(char* arg) {
    int job_number;
    job* j;
    if (arg == NULL || strlen(arg) == 0) {
        j = job_list; 
        while (j) {
            if (j->status == STOP || j->status == RUNNING) break;
            j = j->next;
        }
        if (!j) {
            printf("No stopped jobs to bring to foreground\n");
            return;
        }
    } else {
        job_number = atoi(arg);
        j = find_job_by_number(job_number);
        if (!j || j->status == DONE) {
            printf("Job %d not found\n", job_number);
            return;
        }
    }

    tcsetpgrp(STDIN_FILENO, j->pgid);
    kill(-j->pgid, SIGCONT);
    j->status = RUNNING;
    wait_for_job(j);

    tcsetpgrp(STDIN_FILENO, shell_pgid);
}

void bg(char* arg) {
    int job_number;
    job* j;

    if (arg == NULL || strlen(arg) == 0) {
        j = job_list;
        while (j) {
            if (j->status == STOP) break;
            j = j->next;
        }
        if (!j) {
            printf("No stopped jobs to bring to background\n");
            return;
        }
    } else {
        job_number = atoi(arg);
        j = find_job_by_number(job_number);
        if (!j) {
            printf("Job %d not found\n", job_number);
            return;
        }
    }
    
    kill(-j->pgid, SIGCONT);
    j->status = RUNNING;
}

void jobs() {
    update_job_statuses();
    job* j = job_list;
    while (j) {
        if (strcmp(j->command, "jobs") == 0) {
            j = j->next;
            continue;
        }

        if (j->status == RUNNING) {
            printf("Running   ");
        } else if (j->status == STOP) {
            printf("Stopped   ");
        } else {
            printf("Done      ");
            printf("Job %d: %s\n", j->number, j->command);
            job* next_job = j->next;
            remove_job(j);
            j = next_job;
            continue;
        }

        printf("Job %d: %s\n", j->number, j->command);
        j = j->next;
    }
}

job* find_job_by_number(int number) {
    job* j = job_list;
    while (j) {
        if (j->number == number) {
            return j;
        }
        j = j->next;
    }
    return NULL;
}


void wait_for_job(job* j) {
    int status;
    process* p;
    
    for (p = j->processes; p; p = p->next) {
        if (p->status != DONE) {
            pid_t pid = waitpid(p->pid, &status, WUNTRACED);
            if (pid == -1) {
                perror("waitpid failed");
                exit(EXIT_FAILURE);
            }

            if (WIFSTOPPED(status)) {
                p->status = STOP;
                j->ground = 1;
            } else if (WIFCONTINUED(status)) {
                p->status = RUNNING;
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                p->status = DONE;
            }
        }
    }

    p = j->processes;
    while (p) {
        if (p->status != DONE) {
            j->status = RUNNING;
            return;
        }
        p = p->next;
    }

    j->status = DONE;
    printf("Done      Job %d: %s\n", j->number, j->command);
    remove_job(j);
}
