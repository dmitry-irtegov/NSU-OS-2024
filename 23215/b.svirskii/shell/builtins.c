#include <stdio.h>
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "jobs.h"

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

