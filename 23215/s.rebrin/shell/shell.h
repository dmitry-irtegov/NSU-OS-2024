#include <sys/types.h>
#ifndef SHELL_H
#define SHELL_H

#define MAXARGS 256
#define MAXCMDS 50

struct command {
    char* cmdargs[MAXARGS];
    char cmdflag;
};

/* Command flags */
#define OUTPIP  01
#define INPIP   02

/* External variables */
extern struct command cmds[];
extern char* infile, * outfile, * appfile;
extern char bkgrnd;

/* Function declarations */
int parseline(char* line);

void init_jobs();
void reorder_priorities(int last);
int add_job(char* name, pid_t pid, int frnt);
void upd_job();
int to_fg(int job_id);
int after_fg(int job_id);
int to_bg(int job_id);
void print_jobs();
void pr_job(pid_t pid);
void clear_jobs();
pid_t get_g_int(int job_id);
pid_t get_g_ch(char job_id);
int get_job_id(pid_t p);
void sigCHLD(int sig);
void set_default_termios();
void sigSTOP(int sig);

//int promptline(char* prompt, char* line, int sizline);
void free_ss();

void reset_terminal();
//void able_job_control();
void kill_all();

#endif /* SHELL_H */
