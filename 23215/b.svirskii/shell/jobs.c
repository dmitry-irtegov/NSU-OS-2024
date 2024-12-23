#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "jobs.h"

static Job* first_job = NULL;

const char* state_to_str(JobState state) {
    switch (state) {
        case RUNNING:
            return "RUNNING";
        case STOPPED:
            return "STOPPED";
        case DONE:
            return "DONE";
        case SIGNALED:
            return "SIGNALED";
        default:
            return NULL;
    }
}

void update_job_state(Job* job) {
#ifdef DEBUG
    fprintf(stderr, "(dev) %d - start updating state\n", job->number);
#endif
    if (job->is_foreground || job->state == DONE) {
        return;
    } 
    JobState prev_state = job->state;
    int stat = 0;
    switch (waitpid(-job->pgid, &stat, WNOHANG | WUNTRACED | WCONTINUED)) {
        case -1: 
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
            break;
        case 0: break;
        default:
            if (WIFCONTINUED(stat)) {
                job->state = RUNNING;
            } else if (WIFEXITED(stat)) {
                job->alive_procs_count--;
            } else if (WIFSTOPPED(stat)) {
                job->state = STOPPED;
            } else if (WIFSIGNALED(stat)) {
                job->state = SIGNALED;
            }
    } 
    if (job->alive_procs_count == 0) {
        job->state = DONE;
    }
    if (prev_state != job->state) {
        job->is_state_changed = 1;
    }
#ifdef DEBUG
    fprintf(stderr, "(dev) end\n");
#endif
}

void update_job_states() {
    Job* curr_job = first_job;
    while (curr_job) {
        update_job_state(curr_job);
        curr_job = curr_job->next;
    }
}


void print_job(Job* job) {
    if (job->is_foreground) return;
    job->is_state_changed = 0;
    fprintf(stderr, 
            "[%d] %8s %7d %s\n", job->number, state_to_str(job->state),
                job->pgid, job->name);  
}

Job* delete_job(Job* job) {
    Job* next = job->next;
#ifdef DEBUG
    fprintf(stderr, "(dev) start deleting job %d\n", job->number);
#endif
    if (job->prev) {
        job->prev->next = job->next;
    }
    if (job == first_job) {
        first_job = job->next;
    }
    if (job->next) {
        job->next->prev = job->prev;
    }
    free(job->name);
    free(job);
#ifdef DEBUG
    fprintf(stderr, "(dev) end\n");
#endif
    return next;
}

void print_jobs() {
    Job* curr_job = first_job;
    while (curr_job != NULL) {
        update_job_state(curr_job);
        print_job(curr_job);
        if (curr_job->state == DONE ) {
            curr_job = delete_job(curr_job);
        } else {
            curr_job = curr_job->next;
        }
    }
}

Job* create_job(int pgid, JobState state, char* name, int procs_count) {
    int job_number = 1;
    Job* new_job = (Job*) calloc(1, sizeof(Job));
    
    Job* curr_job = first_job;
    Job* prev_job = NULL;
    while (curr_job) {
        if (curr_job->number == job_number) {
            job_number++;
        } else {
            break;
        }
        prev_job = curr_job;
        curr_job = curr_job->next;
    }
    if (prev_job == NULL) {
        first_job = new_job;
    } else {
        prev_job->next = new_job;
        new_job->prev = prev_job;
    }
    if (curr_job != NULL) {
        new_job->next = curr_job;
        curr_job->prev = new_job;
    }

    int len = strlen(name) + 1;
    new_job->name = (char*) malloc(sizeof(char) * len);
    memcpy(new_job->name, name, len);
    new_job->alive_procs_count = procs_count;
    new_job->number = job_number;
    new_job->pgid = pgid;
    new_job->state = state;
    return new_job;
}

void check_jobs_states_updates() {
#ifdef DEBUG
    fprintf(stderr, "(dev) start checking jobs states updates\n");
#endif
    Job* curr_job = first_job;
    while (curr_job != NULL) {
        update_job_state(curr_job);
        if (curr_job->is_state_changed) {
            print_job(curr_job);
        }
#ifdef DEBUG
        fprintf(stderr, "%d - %s, %d\n", curr_job->number, 
                state_to_str(curr_job->state), curr_job->alive_procs_count);
#endif
        if (curr_job->state == DONE) {
            curr_job = delete_job(curr_job);
        } else {
            curr_job = curr_job->next;
        }
    }
#ifdef DEBUG
    fprintf(stderr, "(dev) end\n");
#endif
}

Job* get_job(JobID id_type, int id) {
    if (first_job == NULL) {
        fprintf(stderr, "no available jobs\n");
        return NULL;
    }

    Job* job = first_job;

    while (job) {
        if (id_type == NUMBER && job->number == id)
            break;
        job = job->next;
    }

    if (job == NULL) {
        fprintf(stderr, "invalid job number\n");
        return NULL;
    }
    return job;
}

void turn_to_foreground(Job* job) {
#ifdef DEBUG
    fprintf(stderr, "(dev) %d turned to fg\n", job->number);
#endif
    if (job == NULL) {
        return;
    }

    if (tcsetpgrp(0, job->pgid) == -1) {
        perror("tcsetpgrp() fail");
        exit(EXIT_FAILURE);
    }

    if (kill(-job->pgid, SIGCONT) == -1) {
        perror("kill() failed");
        exit(EXIT_FAILURE);
    }
    job->state = RUNNING;
    job->is_foreground = 1;
}

void turn_to_background(Job* job) {
#ifdef DEBUG
    fprintf(stderr, "(dev) %d turned to bg\n", job->number);
#endif
    if (job == NULL) {
        return;
    }

    if (tcsetpgrp(0, getpgrp()) == -1) {
        perror("tcsetpgrp() failed");
        exit(EXIT_FAILURE);
    }

    if (kill(-job->pgid, SIGCONT) == -1) {
        perror("kill() failed");
        exit(EXIT_FAILURE);
    }
    job->state = RUNNING;
    job->is_foreground = 0;
}

void stop_job(Job* job) {
    job->state = STOPPED;
    job->is_foreground = 0;
}

Job* get_first_job() {
    return first_job;
}

void destroy_jobs() {
    Job* curr_job = first_job;
    while (curr_job) {
        update_job_state(curr_job);
        if (curr_job->state != DONE) {
            kill(-curr_job->pgid, SIGKILL);
        }
        curr_job = delete_job(curr_job);
    }
    first_job = NULL;
}

void wait_job_in_fg(Job* job) {
    job->is_foreground = 1;
    int stat;
    while (job->alive_procs_count) {
        if (waitpid(-job->pgid, &stat, WUNTRACED | WCONTINUED) == -1) {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(stat)) {
            job->alive_procs_count--;
        } else if (WIFSTOPPED(stat)) {
            job->state = STOPPED;
            job->is_state_changed = 1;
            job->is_foreground = 0;
            return;
        } else if (WIFSIGNALED(stat)) {
            break;
        }
    }
    delete_job(job);
}
