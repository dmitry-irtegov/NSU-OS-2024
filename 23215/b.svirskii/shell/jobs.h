#define MAX_JOB_STR (100)

typedef enum {
    RUNNING,
    STOPPED,
    SIGNALED,
    DONE 
} JobState;

typedef struct Job_t {
    int number;
    int pgid;
    JobState state;
    struct Job_t* prev, *next;
    unsigned char is_state_changed;
    unsigned char is_foreground;
    int alive_procs_count;
    char* name;
} Job;

typedef enum {
    NUMBER
} JobID;

Job* create_job(int pgid, JobState state, char* promptline, int procs_count);
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
