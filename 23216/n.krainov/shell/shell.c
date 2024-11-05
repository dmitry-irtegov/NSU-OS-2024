#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include "shell.h"

int initShellPreferences() {
    if (!isatty(STDIN_FILENO)) {
        return -1;
    }

    //rsigset(SIGINT, SIG_IGN);
    sigset(SIGTSTP, SIG_IGN);
    sigset(SIGQUIT, SIG_IGN);
    sigset(SIGTTOU, SIG_IGN);
    sigset(SIGTTIN, SIG_IGN);

    pid_t pid = getpid();
    if (setpgid(pid, pid) < 0) {
        return 1;
    }

    if (tcsetpgrp(STDIN_FILENO, pid)) {
        return 1;
    }
    return 0;
}

int main() {
    char line[1024];
    
    if (initShellPreferences()) {
        perror("initShellPreferences failed");
        exit(EXIT_FAILURE);
    }
    
    Conv* conv = NULL;
    
    while (promptline(line, sizeof(line)) > 0) {   
        conv = calloc(1, sizeof(Conv));
        if (parseline(line, conv) == -1 || conv->cmd == NULL) {
            updateInfoJobs(0);
            freeSpace(conv);
            free(conv);
            continue;
        }  
        createJobs(conv);
        freeSpace(conv);
        free(conv);
        updateInfoJobs(0);
    }  

    exit(EXIT_SUCCESS);
}