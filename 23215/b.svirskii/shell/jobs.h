#include <sys/types.h>
#define MAX_JOB_STR (100)

typedef enum {
    RUNNING,
    STOPPED,
    SIGNALED,
    DONE 
} JobState;

void create_job(pid_t pid, JobState state);

void print_jobs();

void check_jobs_states_updates();
