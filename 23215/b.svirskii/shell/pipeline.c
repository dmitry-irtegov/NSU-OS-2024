#include "shell.h"
#include "jobs.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int curr_pipe_fds[2] = { -1, -1 };
static int prev_pipe_out_fd = -1;
static int curr_pipe_pgid = 0;
static Proc* curr_pipe_procs = NULL;
static char curr_pipeline_prompt[1024] = "";

unsigned char is_pipe(struct command cmd) {
    return cmd.cmdflag & INPIP || cmd.cmdflag & OUTPIP;
}

unsigned char is_inpipe(struct command cmd) {
    return cmd.cmdflag & INPIP;
}

unsigned char is_outpipe(struct command cmd) {
    return cmd.cmdflag & OUTPIP;
}

int get_pipeline_in_fd() {
    return curr_pipe_fds[1];
}

int get_pipeline_out_fd() {
    return prev_pipe_out_fd;
}

void create_new_pipe() {
    if (pipe(curr_pipe_fds) == -1) {
        perror("pipe() failed");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    fprintf(stderr, "(dev) new pipe created: %d %d\n", curr_pipe_fds[0], 
            curr_pipe_fds[1]);
#endif
}

void prepare_for_next_pipe() {
    close(prev_pipe_out_fd);
    close(curr_pipe_fds[1]);
    prev_pipe_out_fd = curr_pipe_fds[0];
    curr_pipe_fds[0] = curr_pipe_fds[1] = -1;
}

void end_pipeline() {
    curr_pipe_pgid = 0;
    curr_pipe_procs = NULL;
    close(prev_pipe_out_fd);
    prev_pipe_out_fd = -1;
    curr_pipeline_prompt[0] = '\0';
    prev_pipe_out_fd = curr_pipe_fds[0] = curr_pipe_fds[1] = -1;
}

void add_proc_to_pipeline(Proc* proc) {
    setpgid(proc->pid, curr_pipe_pgid);
    if (curr_pipe_procs != NULL) {
        strcat(curr_pipeline_prompt, " | ");
    } else {
        curr_pipe_pgid = getpgid(proc->pid);
    }
    strcat(curr_pipeline_prompt, proc->prompt); 
    proc->next = curr_pipe_procs;
    curr_pipe_procs = proc;
}

void close_all_pipeline_fds() {
    close(prev_pipe_out_fd);
    prev_pipe_out_fd = -1;
    close(curr_pipe_fds[0]);
    close(curr_pipe_fds[1]);
    curr_pipe_fds[0] = curr_pipe_fds[1] = -1;
}

char* get_pipeline_prompt() {
    return curr_pipeline_prompt;
}

Proc* get_pipeline_procs() {
    return curr_pipe_procs;
}
