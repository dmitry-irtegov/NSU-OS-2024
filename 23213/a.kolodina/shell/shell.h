#include <sys/types.h>
#include <termios.h>

#define MAXARGS 256
#define MAXCMDS 50
struct command {
  char *cmdargs[MAXARGS];
  char cmdflag;
};

typedef struct process {
  struct process* prev;
  struct process* next; 
  pid_t p_pid; 
  pid_t g_pid; 
  struct command argv;
  int status; // 0 - terminated, 1 - active, 2 - stopped.
  int fildes[2];
  int prev_fildes[2];
  int bg; // 0 - fg, 1 -bg
  char* infile;
  char* outfile;
  char* appfile;
} process;

typedef struct job {
  int idx;
  struct job* prev;
  struct job* next;
  pid_t job_pgid; 
  process* processes;
  int status;  //0 - terminated, 1 - active, 2 - stopped.
  int fildes[2];
  int bg; // 0 - fg, 1 -bg
  char* job_name;
  char cmdflag;
  char* infile;
  char* outfile;
  char* appfile;
} job;

typedef struct shell_command {
  job* j;
  char* shell_cmd[2];
} shell_command;


 /*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02
  
extern struct command cmds[];
extern job* bg_jobs;
extern int bg_jobs_number;
extern int biggest_idx;
extern pid_t shell;
extern struct termios shell_modes;
extern char prompt[]; 
extern int is_interactive;
extern FILE* script;
job* parseline(char *);
int promptline(char *, char *, int);

void start_process(process* p);
void start_job(job* job);
void wait_for_job(job* job);
void fg(char* arg);
void bg(char* arg);
void jobs();
void free_process(process* p);
void free_job(job* j);
void sigint_handler(int sig);
void monitor_jobs();
job* find_job(pid_t gpid);
void remove_job(job* j);
void build_job(job* j, struct command* cmds, int ncmds, int curr_idx);
