#pragma once
#define MAX_PROMPT_LEN (1024)

typedef enum {
    RUNNING,
    STOPPED,
    SIGNALED,
    DONE 
} State;

typedef struct Proc_t {
    int pid;
    struct Proc_t *next;
    State state;
    char prompt[MAX_PROMPT_LEN];
} Proc;

typedef struct Job_t {
    int number;
    int pgid;
    State state;
    struct Job_t* prev, *next;
    unsigned char is_state_changed;
    unsigned char is_foreground;
    char prompt[MAX_PROMPT_LEN];
    Proc* procs;
} Job;

typedef enum {
    NUMBER
} JobID;

Job* create_job(int pgid, State state, char* prompt, Proc* procs);
void print_job(Job* job);
void print_jobs();
void update_job_states();
void check_jobs_states_updates();
Job* get_job(JobID id_type, int job_number);
void turn_to_background(Job* job);
void turn_to_foreground(Job* job);
void stop_job(Job* job);
Job* get_first_job();
__attribute__((destructor))
void destroy_jobs();
void wait_job_in_fg(Job* job);
Job* parse_job(char* line);
