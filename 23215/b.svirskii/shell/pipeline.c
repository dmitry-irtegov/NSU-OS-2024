#include "shell.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int curr_pipe_fds[2] = { -1, -1 };
static int prev_pipe_out_fd = -1;
static int curr_pipe_pgid = 0;
static int curr_pipeline_procs_count = 0;
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
    fprintf(stderr, "new pipe created: %d %d\n", curr_pipe_fds[0], 
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
    curr_pipeline_procs_count = 0;
    curr_pipe_pgid = 0;
    close(prev_pipe_out_fd);
    prev_pipe_out_fd = -1;
    curr_pipeline_prompt[0] = '\0';
    prev_pipe_out_fd = curr_pipe_fds[0] = curr_pipe_fds[1] = -1;
}

void add_proc_to_pipeline(int pid, char* cmd_prompt) {
    setpgid(pid, curr_pipe_pgid);
    if (curr_pipeline_procs_count > 0) {
        strcat(curr_pipeline_prompt, " | ");
    } else {
        curr_pipe_pgid = getpgid(pid);
    }
    strcat(curr_pipeline_prompt, cmd_prompt); 
    curr_pipeline_procs_count++;
}

void close_all_pipeline_fds() {
    close(prev_pipe_out_fd);
    prev_pipe_out_fd = -1;
    close(curr_pipe_fds[0]);
    close(curr_pipe_fds[1]);
    curr_pipe_fds[0] = curr_pipe_fds[1] = -1;
}

int get_pipeline_procs_count() {
    return curr_pipeline_procs_count;
}

char* get_pipeline_prompt() {
    return curr_pipeline_prompt;
}
