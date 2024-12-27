#include "jobs.h"

#define MAXARGS 256
#define MAXCMDS 50

struct command {
    char *cmdargs[MAXARGS];
    char cmdflag;
};

/*  cmdflag's  */
#define OUTPIP  01
#define INPIP   02

extern struct command cmds[];
extern char *infile, *outfile, *appfile;
extern char bkgrnd;

int parseline(char *);
int promptline(char *, char *, int);

// pipelines
unsigned char is_pipe(struct command cmd);
unsigned char is_inpipe(struct command cmd);
unsigned char is_outpipe(struct command cmd);
int get_pipeline_in_fd();
int get_pipeline_out_fd();
void create_new_pipe();
void prepare_for_next_pipe();
void end_pipeline();
void add_proc_to_pipeline(Proc* proc);
void close_all_pipeline_fds();
int get_pipeline_procs_count();
char* get_pipeline_prompt();
Proc* get_pipeline_procs();

// builtins
unsigned char run_builtin(char** args);

// runner
void prepare_in_fd(struct command cmd);
void prepare_out_fd(struct command cmd);
void run_proc(struct command cmd);

// utils
void build_cmd_prompt(struct command cmd, char* buff);
void ignore_signals();
void set_sighandlers_to_dfl();
Proc* create_proc(int pid, char* prompt);
