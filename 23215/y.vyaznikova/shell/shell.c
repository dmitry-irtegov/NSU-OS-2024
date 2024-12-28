/**
 * @file shell.c
 * @brief Main shell implementation file
 *
 * This file contains the core functionality of the shell, including:
 * - Shell initialization and terminal setup
 * - Signal handling
 * - Command line processing
 * - Prompt customization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <termios.h>
#include <pwd.h>
#include "shell.h"
#include "jobs.h"
#include "builtins.h"

/* Global variables */
char prompt[PROMPTLEN];           // Shell prompt string
pid_t shell;                      // Shell process ID
struct termios shell_modes;       // Terminal modes for the shell
job* bg_jobs = NULL;             // List of background jobs
int bg_jobs_number = 0;          // Number of background jobs
int biggest_idx = 0;             // Largest job index used
char cwd[1025];                  // Current working directory
FILE* script = 0;                // Script file if shell is running a script
int is_interactive = 0;          // Flag indicating if shell is interactive

/* Helper functions for shell prompt initialization */

/**
 * @brief Get the username for the current user
 * 
 * @param username Buffer to store the username
 * @param size Size of the username buffer
 * @return char* Pointer to the username buffer
 */
static char* get_username(char* username, size_t size) {
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw != NULL) {
        strncpy(username, pw->pw_name, size - 1);
        username[size - 1] = '\0';
    } else {
        strncpy(username, "user", size - 1);
        username[size - 1] = '\0';
    }
    return username;
}

/**
 * @brief Get the hostname of the system
 * 
 * @param hostname Buffer to store the hostname
 * @param size Size of the hostname buffer
 * @return char* Pointer to the hostname buffer
 */
static char* get_hostname(char* hostname, size_t size) {
    if (gethostname(hostname, size) != 0) {
        strncpy(hostname, "localhost", size - 1);
    }
    hostname[size - 1] = '\0';
    return hostname;
}

/**
 * @brief Format the current path, replacing home directory with ~
 * 
 * @param result Buffer to store the formatted path
 * @param size Size of the result buffer
 * @param cwd Current working directory path
 */
static void format_path_with_home(char* result, size_t size, const char* cwd) {
    char* home = getenv("HOME");
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(result, size, "~%s", cwd + strlen(home));
    } else {
        strncpy(result, cwd, size - 1);
        result[size - 1] = '\0';
    }
}

/**
 * @brief Initialize the shell prompt with username, hostname and current directory
 * 
 * Creates a prompt in the format: username@hostname:path%
 * The path is shortened by replacing the home directory with ~
 * 
 * @param prompt Buffer to store the formatted prompt
 */
void initialize_shell_prompt(char prompt[]) {
    char hostname[64] = {0};
    char username[32] = {0};
    char current_dir[PROMPTLEN] = {0};
    char formatted_path[PROMPTLEN-100] = {0};  // Reserve space for other parts
    
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        strcpy(prompt, "% ");
        return;
    }
    
    get_username(username, sizeof(username));
    get_hostname(hostname, sizeof(hostname));
    format_path_with_home(formatted_path, sizeof(formatted_path), current_dir);
    
    snprintf(prompt, PROMPTLEN, "%s@%s:%s%% ",
            username, hostname, formatted_path);
}

/**
 * @brief Handle keyboard interrupt (Ctrl+C)
 * 
 * Prints a newline and redisplays the prompt when Ctrl+C is pressed
 * 
 * @param sig Signal number (unused)
 */
void handle_keyboard_interrupt(int sig __attribute__((unused))) {
    write(STDOUT_FILENO, "\n", strlen("\n"));
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

/**
 * @brief Handle keyboard quit signal (Ctrl+\)
 * 
 * Displays a newline and redisplays the prompt when Ctrl+\ is pressed
 * 
 * @param sig Signal number (unused)
 */
void handle_keyboard_quit(int sig __attribute__((unused))) {
    write(STDOUT_FILENO, "\n", strlen("\n"));
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

/**
 * @brief Handle keyboard suspend signal (Ctrl+Z)
 * 
 * Displays a newline and redisplays the prompt when Ctrl+Z is pressed
 * 
 * @param sig Signal number (unused)
 */
void handle_keyboard_suspend(int sig __attribute__((unused))) {
    write(STDOUT_FILENO, "\n", strlen("\n"));
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

/* Helper functions for shell initialization and command processing */

/**
 * @brief Initialize signal handlers for the shell
 * 
 * Sets up handlers for:
 * - SIGTTIN, SIGTTOU (terminal input/output)
 * - SIGQUIT (Ctrl+\)
 * - SIGTSTP (Ctrl+Z)
 * - SIGINT (Ctrl+C)
 */
static void initialize_shell_signals(void) {
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    
    struct sigaction sa_int, sa_quit, sa_tstp;
    
    // Setup for SIGINT (Ctrl+C)
    sa_int.sa_handler = handle_keyboard_interrupt;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);
    
    // Setup for SIGQUIT (Ctrl+\)
    sa_quit.sa_handler = handle_keyboard_quit;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &sa_quit, NULL);
    
    // Setup for SIGTSTP (Ctrl+Z)
    sa_tstp.sa_handler = handle_keyboard_suspend;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}

/**
 * @brief Initialize the shell's terminal settings
 * 
 * Sets up the shell's process group and terminal modes.
 * Ensures the shell is in the foreground if it's interactive.
 */
static void initialize_shell_terminal(void) {
    shell = getpid();
    setpgid(shell, shell);
    
    if (is_interactive && tcsetpgrp(STDIN_FILENO, shell) == -1) {
        perror("error tcsetpgrp shell");
        exit(1);
    }
    
    if (is_interactive && tcgetattr(STDIN_FILENO, &shell_modes) == -1) {
        perror("tcgettatrr error");
        exit(1);
    }
}

/**
 * @brief Check if the command line contains the exit command
 * 
 * @param line Command line to check
 * @return int 1 if exit command is found, 0 otherwise
 */
static int check_exit_command(const char* line) {
    const char* cmd = line;
    // Skip leading whitespace
    while (*cmd == ' ' || *cmd == '\t') cmd++;
    if (strncmp(cmd, "exit", 4) == 0) {
        char *p = (char*)cmd + 4;
        // Skip trailing whitespace
        while (*p == ' ' || *p == '\t') p++;
        // Check if nothing follows exit command
        if (*p == '\n' || *p == '\0') {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Process a command line input
 * 
 * Handles:
 * - Empty commands
 * - Exit command
 * - Command parsing and execution
 * - Job monitoring
 * 
 * @param line Command line to process
 * @return int 1 if shell should exit, 0 otherwise
 */
static int process_command_line(const char* line) {
    // Handle empty command
    if (line[0] == '\0') {
        monitor_jobs();
        return 0;
    }
    
    // Check for exit command
    if (check_exit_command(line)) {
        return 1;
    }
    
    // Parse and execute command
    job* j = parseline((char*)line);
    if (!j) {
        monitor_jobs();
        return 0;
    }

    // Execute each job in the command line
    job* curr;
    while (j) {
        curr = j;
        j = j->next;
        start_job(curr);
    }
    monitor_jobs();
    return 0;
}

/**
 * @brief Main shell function
 * 
 * Initializes the shell environment and runs the main command loop.
 * Handles:
 * - Script mode vs interactive mode
 * - Shell initialization
 * - Command line reading and processing
 * 
 * @param argc Argument count (unused)
 * @param argv Argument vector
 * @return int Exit status
 */
int main(int argc __attribute__((unused)), char *argv[]) {
    // Check for script mode
    if (argv[1]) {
        script = fopen(argv[1], "r");
    }
    
    // Check if shell is interactive
    is_interactive = isatty(STDIN_FILENO);
    
    // Initialize shell environment
    initialize_shell_terminal();
    initialize_shell_signals();
    initialize_shell_prompt(prompt);

    // Main command loop
    char line[1024];   
    int read_result;
    while ((read_result = promptline(prompt, line, sizeof(line))) >= 0) { 
        if (process_command_line(line)) {
            return 0;  // Exit command received - clean exit
        }
    }
    
    // Add newline only if we exited due to EOF (Ctrl+D)
    if (read_result == -1) {
        write(STDOUT_FILENO, "\n", 1);
    }
    
    return 0;
}