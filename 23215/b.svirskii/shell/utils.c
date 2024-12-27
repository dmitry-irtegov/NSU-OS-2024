#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "shell.h"
#include "jobs.h"
#define IGNORING_SIGNALS_COUNT (5)

void build_cmd_prompt(struct command cmd, char* buff) {
    for (char** arg = cmd.cmdargs; *arg; arg++) {
        strcat(buff, *arg);
        if (*(arg + 1) != NULL) {
            strcat(buff, " ");
        }
    }
}

static int signals[IGNORING_SIGNALS_COUNT] = { SIGINT, SIGQUIT, SIGTTOU, 
    SIGTTIN, SIGTSTP };

void ignore_signals() {
    for (int i = 0; i < IGNORING_SIGNALS_COUNT; i++) {
       signal(signals[i], SIG_IGN); 
    }
}

void set_sighandlers_to_dfl() {
    for (int i = 0; i < IGNORING_SIGNALS_COUNT; i++) {
       signal(signals[i], SIG_DFL); 
    }
    signal(SIGCHLD, SIG_DFL);
}

Proc* create_proc(int pid, char* prompt) {
    Proc* proc = (Proc*) calloc(1, sizeof(Proc));
    proc->pid = pid;
    proc->state = RUNNING;
    strcpy(proc->prompt, prompt);
    return proc;
}
