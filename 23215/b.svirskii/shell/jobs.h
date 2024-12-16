#include <sys/types.h>
#define MAX_JOB_STR (100)

typedef enum {
    RUNNING,
    STOPPED,
    SIGNALED,
    DONE 
} JobState;

typedef struct Job_t {
    int number;
    pid_t pid;
    JobState state;
    struct Job_t* prev, *next;
    unsigned char is_state_changed;
    unsigned char is_foreground;
    char* name;
} Job;

typedef enum {
    NUMBER,
    PID
} JobID;

void create_job(pid_t pid, JobState state, char* name);
void print_jobs();
void check_jobs_states_updates();
Job* get_job(JobID id_type, int job_number);
void turn_to_background(Job* job);
void turn_to_foreground(Job* job);
void stop_job(Job* job);
Job* get_first_job();
