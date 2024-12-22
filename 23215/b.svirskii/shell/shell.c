#include <limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "shell.h"
#include <signal.h>
#include "jobs.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

//TODO
char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

Job* parse_job(char* line) {
    if (line == NULL) {
        fprintf(stderr, "can't identify job\n");
        return NULL;
    } else if (line[0] == '%') {
        if (line[1] == '\0') {
            if (get_first_job() == NULL) {
                fprintf(stderr, "no available jobs\n");
                return NULL;
            }
            return get_first_job();
        }
        int job_number = atoi(line + 1);
        if (job_number < 1) {
            fprintf(stderr, "invalid job number\n");
            return NULL;
        }
        return get_job(NUMBER, job_number);
    }
    return NULL;
}

unsigned char run_builtin(char** args) {
    if (strcmp("jobs", args[0]) == 0) {
        print_jobs();
        return 1;
    }
    if (strcmp("fg", args[0]) == 0) {
        Job* job = parse_job(args[1]);   
        if (job == NULL) return 1;
        turn_to_foreground(job);

        wait_job_in_fg(job);

        if (tcsetpgrp(0, getpgrp()) == -1) {
            perror("tcsetpgrp() failed");
            exit(EXIT_FAILURE);
        }
        return 1;
    }
    if (strcmp("bg", args[0]) == 0) {
        Job* job = parse_job(args[1]);   
        turn_to_background(job);
        return 1; 
    }
    if (strcmp("cd", args[0]) == 0) {
        if (args[1] == NULL) {
            args[1] = getenv("HOME");
        }
        if (chdir(args[1]) == -1) {
            perror("cd");
        }
        return 1;
    }
    return 0;
}

unsigned char is_pipe(struct command cmd) {
    return cmd.cmdflag & INPIP || cmd.cmdflag & OUTPIP;
}

unsigned char is_inpipe(struct command cmd) {
    return cmd.cmdflag & INPIP;
}

unsigned char is_outpipe(struct command cmd) {
    return cmd.cmdflag & OUTPIP;
}

void sigchld_hand(int sig) {
    update_job_states();
}

int main(int argc, char *argv[])
{
    if (!isatty(0)) {
        fprintf(stderr, "stdin doesn't associated with terminal\n");
        exit(EXIT_FAILURE);
    }
    if (!isatty(1)) {
        fprintf(stderr, "stdout doesn't associated with terminal\n");
        exit(EXIT_FAILURE);
    }
    register int i;
    char line[1024] = "";      /*  allow large command lines  */
    int ncmds = 0;
    char prompt[50] = "";      /* shell prompt */
    int child_pid = -1;
    int curr_pipe_fds[2] = { -1, -1 };
    int prev_pipe_out_fd = -1;
    int curr_pipe_pgid = 0;
    int curr_procs_count = 0;
    char curr_cmd[1024] = "";

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCHLD, sigchld_hand);

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) {    /* until eof  */
        if ((ncmds = parseline(line)) <= 0) {
            check_jobs_states_updates();
            continue;   /* read next line */
        }
#ifdef DEBUG
{
    for (int i = 0; i < ncmds; i++) {
        for (int j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++) {
            fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n", 
                    i, j, cmds[i].cmdargs[j]);
        }
        fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
    }
    fprintf(stderr, "infile: %s\n", infile);
    fprintf(stderr, "outfile: %s\n", outfile);
    fprintf(stderr, "appfile: %s\n", appfile);
}
#endif
        for (i = 0; i < ncmds; i++) {
            if (run_builtin(cmds[i].cmdargs)) {
                continue;
            }
            if (is_outpipe(cmds[i])) {
                if (pipe(curr_pipe_fds) == -1) {
                    perror("pipe() failed");
                    exit(EXIT_FAILURE);
                }
            }
            child_pid = fork();
            if (child_pid == -1) {
                perror("fork() failed");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) {
                mode_t created_file_mode = S_IWRITE | S_IREAD | S_IRGRP 
                    | S_IWGRP;
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);
                if (is_inpipe(cmds[i])) {
                    close(0);
                    dup(prev_pipe_out_fd);
                    close(prev_pipe_out_fd);
                } else if (i == 0 && infile) {
                    close(0);
                    if (open(infile, O_RDONLY) == -1) {
                        perror("open() input file failed");
                        exit(EXIT_FAILURE);
                    }
                }
                if (is_outpipe(cmds[i])) {
                    close(1);
                    dup(curr_pipe_fds[1]);
                    close(curr_pipe_fds[0]);
                    close(curr_pipe_fds[1]);
                } else if (i == ncmds - 1 && outfile) {
                    close(1);
                    if (open(outfile, O_CREAT | O_WRONLY | O_TRUNC,
                                created_file_mode) == -1) {
                        perror("open() output file failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (i == ncmds - 1 && appfile) {
                    close(1);
                    if (open(appfile, O_CREAT | O_WRONLY | O_APPEND, 
                                created_file_mode) == -1) {
                        perror("open() append file failed");
                        exit(EXIT_FAILURE);
                    } 
                }

                if (execvp(cmds[i].cmdargs[0], cmds[i].cmdargs) == -1) {
                    perror(cmds[i].cmdargs[0]);
                    close(0);
                    close(1);
                    exit(EXIT_FAILURE);
                }
                close(0);
                close(1);
                exit(EXIT_SUCCESS);
            } else {
                if (is_pipe(cmds[i])) {
                    curr_procs_count++;
                    close(prev_pipe_out_fd);
                    close(curr_pipe_fds[1]);
                    prev_pipe_out_fd = curr_pipe_fds[0];
                    curr_pipe_fds[0] = curr_pipe_fds[1] = -1;
                }

                for (char** arg = cmds[i].cmdargs; *arg; arg++) {
                    strcat(curr_cmd, *arg);
                    strcat(curr_cmd, " ");
                }

                if (is_pipe(cmds[i])) {
                    if (cmds[i].cmdflag & OUTPIP) {
                        strcat(curr_cmd, "| ");
                    }
                    setpgid(child_pid, curr_pipe_pgid);
                    if (curr_pipe_pgid == 0) {
                        curr_pipe_pgid = getpgid(child_pid);
                    }
                } else {
                    setpgid(child_pid, 0);
                }
                if (is_outpipe(cmds[i])) {
                    continue;
                } // otherwise it's the last proc in pipe
                Job* job = create_job(getpgid(child_pid), RUNNING, curr_cmd,
                                is_pipe(cmds[i]) ? curr_procs_count : 1);
                if (bkgrnd) {
                    print_job(job);
                } else {
                    tcsetpgrp(0, job->pgid);
                    wait_job_in_fg(job);
                    tcsetpgrp(0, getpgrp());
                }
            }
            if (!is_outpipe(cmds[i])) { // last proc in pipe or not a pipe
                *curr_cmd = '\0';
                if (is_inpipe(cmds[i])) {
                    curr_procs_count = 0;
                    curr_pipe_pgid = 0;
                    prev_pipe_out_fd = -1;
                }
            }
        }
        check_jobs_states_updates();
    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */  
