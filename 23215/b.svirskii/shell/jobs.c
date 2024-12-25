#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "jobs.h"

static Job* first_job = NULL;

const char* state_to_str(State state) {
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

int is_dead(State state) {
	return state == DONE || state == SIGNALED;
}

void update_job_state(Job* job) {
    if (job->is_foreground || is_dead(job->state)) {
        return;
    } 
#ifdef DEBUG
    fprintf(stderr, "(dev) %d - start updating state\n", job->number);
#endif
    State prev_state = job->state;
    int stat;
    int is_all_done = 1;
    int is_all_signaled = 1;
    int is_all_running = 1;
    int is_all_dead = 1;
    for (Proc* curr_proc = job->procs; curr_proc; curr_proc = curr_proc->next) {
        if (is_dead(curr_proc->state)) {
            continue;
        }
	    stat = 0;
	    switch (waitpid(curr_proc->pid, &stat, 
                    WNOHANG | WUNTRACED | WCONTINUED)) {
            case -1: 
                perror("waitpid() failed");
                exit(EXIT_FAILURE);
                break;
            case 0: break;
            default:
                if (WIFEXITED(stat)) {
                    curr_proc->state = DONE;
                } else if (WIFSIGNALED(stat)) {
                    curr_proc->state = SIGNALED;
                } else if (WIFCONTINUED(stat)) {
                    curr_proc->state = RUNNING;
                } else if (WIFSTOPPED(stat)) {
                    job->state = curr_proc->state = STOPPED;
                }
	    }
	    if (is_all_done && curr_proc->state != DONE) {
		    is_all_done = 0;
	    }
	    if (is_all_signaled && curr_proc->state != SIGNALED) {
		    is_all_signaled = 0;
	    }
	    if (is_all_running && curr_proc->state != RUNNING) {
		    is_all_running = 0;
	    }
        if (is_all_dead && !is_dead(curr_proc->state)) {
            is_all_dead = 0;
        }
    }
    if (is_all_done) {
	    job->state = DONE;
    } else if (is_all_signaled || is_all_dead) {
	    job->state = SIGNALED;
    } else if (is_all_running) {
	    job->state = RUNNING;
    }
    if (prev_state != job->state) {
        job->is_state_changed = 1;
    }
#ifdef DEBUG
    fprintf(stderr, "(dev) end\n");
#endif
}

void print_job(Job* job) {
    if (job->is_foreground) return;
    job->is_state_changed = 0;
    fprintf(stderr, 
            "[%d] %8s %7d %s\n", job->number, state_to_str(job->state),
                job->pgid, job->prompt);  
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
    Proc* prev_proc = NULL;
    Proc* curr_proc = job->procs;
    while (curr_proc) {
        prev_proc = curr_proc;
        curr_proc = curr_proc->next;
        free(prev_proc);
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
        if (is_dead(curr_job->state)) {
            curr_job = delete_job(curr_job);
        } else {
            curr_job = curr_job->next;
        }
    }
}

Job* create_job(int pgid, State state, char* prompt, Proc* procs) {
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
    
    strcpy(new_job->prompt, prompt);
    new_job->procs = procs;
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
        if (is_dead(curr_job->state)) {
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
        if (!is_dead(curr_job->state)) {
            kill(-curr_job->pgid, SIGKILL);
        }
        curr_job = delete_job(curr_job);
    }
    first_job = NULL;
}

void wait_job_in_fg(Job* job) {
#ifdef DEBUG
    fprintf(stderr, "(dev) start waiting job %d in fg\n", job->number);
#endif
    job->is_foreground = 1;
    int stat;
    for (Proc* curr_proc = job->procs; curr_proc; curr_proc = curr_proc->next) {
        if (is_dead(curr_proc->state)) {
            continue;
        }
#ifdef DEBUG
        fprintf(stderr, "(dev) start updating state of \"%s\", pid: %d\n", curr_proc->prompt, curr_proc->pid);
#endif
        stat = 0;
        if (waitpid(curr_proc->pid, &stat, WUNTRACED) == -1) {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(stat)) {
            curr_proc->state = DONE;
        }  else if (WIFSIGNALED(stat)) {
            curr_proc->state = SIGNALED;
        } else {
            if (WIFSTOPPED(stat)) {
                job->state = curr_proc->state = STOPPED;
                job->is_state_changed = 1;
                job->is_foreground = 0;
                return;
            }
        }
    }
    delete_job(job);
}
