#include <limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "shell.h"
#include <stdlib.h>
#include <sys/stat.h>

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

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
    char curr_cmd[1024] = "";

    ignore_signals();

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
                create_new_pipe();
            }
            child_pid = fork();
            if (child_pid == -1) {
                perror("fork() failed");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) {
                prepare_in_fd(cmds[i]);
                prepare_out_fd(cmds[i]);
                close_all_pipeline_fds();
                run_proc(cmds[i]);
            } else {
                build_cmd_prompt(cmds[i], curr_cmd);
                Proc* new_proc = create_proc(child_pid, curr_cmd);

                if (is_pipe(cmds[i])) {
                    prepare_for_next_pipe();
                    add_proc_to_pipeline(new_proc);
                } else {
                    setpgid(child_pid, 0);
                }
                if (is_outpipe(cmds[i])) {
                    *curr_cmd = '\0';
                    continue;
                } // otherwise it's the last proc in pipe

                Job* job = create_job(getpgid(child_pid), RUNNING, 
                        is_pipe(cmds[i]) ? get_pipeline_prompt() : curr_cmd, 
                        is_pipe(cmds[i]) ? get_pipeline_procs() : new_proc);

#ifdef DEBUG
                print_job(job);
#endif

                if (bkgrnd) {
                    print_job(job);
                } else {
                    tcsetpgrp(0, job->pgid);
                    wait_job_in_fg(job);
                    if (tcsetpgrp(0, getpgrp()) == -1) {
                        perror("tcsetpgrp() failed");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            *curr_cmd = '\0';
            if (is_inpipe(cmds[i])) {
                end_pipeline();
            }
        }
        check_jobs_states_updates();
    }  /* close while */
}

/* PLACE SIGNAL CODE HERE */  
