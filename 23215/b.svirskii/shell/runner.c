#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "shell.h"

extern char *infile, *outfile, *appfile;
static mode_t create_file_mode = S_IWRITE | S_IREAD | S_IRGRP | S_IROTH;

void prepare_in_fd(struct command cmd) {
    int in_fd = -1;
    if (is_inpipe(cmd)) {
        in_fd = get_pipeline_out_fd();
    } else if (infile != NULL) {
        if ((in_fd = open(infile, O_RDONLY,
                        create_file_mode)) == -1) {
            perror("open() infile failed");
            exit(EXIT_FAILURE);
        }
    }
    if (in_fd != -1) {
        close(0);
        dup(in_fd);
        if (!is_inpipe(cmd)) {
            close(in_fd);
        }
    }
}

void prepare_out_fd(struct command cmd) {
    int out_fd = -1;
    if (is_outpipe(cmd)) {
        out_fd = get_pipeline_in_fd();
    } else if (outfile != NULL) {
        if ((out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC,
                        create_file_mode)) == -1) {
            perror("open() outfile failed");
            exit(EXIT_FAILURE);
        }
    } else if (appfile != NULL) {
        if ((out_fd = open(appfile, O_WRONLY | O_CREAT | O_APPEND,
                        create_file_mode)) == -1) {
            perror("open() appfile failed");
            exit(EXIT_FAILURE);
        } 
    }
    if (out_fd != -1) {
        close(1);
        dup(out_fd);
        if (!is_outpipe(cmd)) {
            close(out_fd);
        }
    } 
}

void run_proc(struct command cmd) {
    set_sighandlers_to_dfl();

    if (execvp(cmd.cmdargs[0], cmd.cmdargs) == -1) {
        perror(cmd.cmdargs[0]);
        close(0);
        close(1);
        exit(EXIT_FAILURE);
    }
    close(0);
    close(1);
    exit(EXIT_SUCCESS); 
}

