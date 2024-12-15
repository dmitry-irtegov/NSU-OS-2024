#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct Job_t {
    int number;
    pid_t pid;
    JobState state;
    struct Job_t* prev, *next;
    char is_state_changed;
} Job;

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
    if (job->state == DONE) {
        return;
    }
    JobState prev_state = job->state;
    int stat = 0;
    switch (waitpid(job->pid, &stat, WNOHANG | WUNTRACED | WCONTINUED)) {
        case -1:
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
            break;
        case 0: break;
        default:
            if (WIFCONTINUED(stat)) {
                job->state = RUNNING;
            } else if (WIFEXITED(stat)) {
                job->state = DONE;
            } else if (WIFSTOPPED(stat)) {
                job->state = STOPPED;
            } else if (WIFSIGNALED(stat)) {
                job->state = SIGNALED;
            }
            if (prev_state != job->state) {
                job->is_state_changed = 1;
            }
    } 
#ifdef DEBUG
    fprintf(stderr, "(dev) end\n");
#endif
}


void print_job(Job* job) {
    job->is_state_changed = 0;
    fprintf(stderr, 
            "[%d] %8s %7d\n", job->number, state_to_str(job->state), job->pid);  
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
        if (curr_job->state == DONE) {
            curr_job = delete_job(curr_job);
        } else {
            curr_job = curr_job->next;
        }
    }
}

void create_job(pid_t pid, JobState state) {
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

    new_job->number = job_number;
    new_job->pid = pid;
    new_job->state = state;
    print_job(new_job);
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
