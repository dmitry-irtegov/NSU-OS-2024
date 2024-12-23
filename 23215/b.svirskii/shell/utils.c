#include <string.h>
#include <signal.h>
#include "shell.h"
#define IGNORING_SIGNALS_COUNT (6)

void build_cmd_prompt(struct command cmd, char* buff) {
    for (char** arg = cmd.cmdargs; *arg; arg++) {
        strcat(buff, *arg);
        strcat(buff, " ");
    }
}

static int signals[IGNORING_SIGNALS_COUNT] = { SIGINT, SIGQUIT, SIGTTOU, 
    SIGTTIN, SIGTSTP, SIGCHLD };

void ignore_signals() {
    for (int i = 0; i < IGNORING_SIGNALS_COUNT; i++) {
       signal(signals[i], SIG_IGN); 
    }
}

void set_sighandlers_to_dfl() {
    for (int i = 0; i < IGNORING_SIGNALS_COUNT; i++) {
       signal(signals[i], SIG_DFL); 
    }
}
