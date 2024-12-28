/**
 * @file shell.h
 * @brief Main header file for the shell implementation
 *
 * This header defines the core structures and functions for the shell,
 * including command processing, job control, and terminal handling.
 */

#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>
#include <termios.h>
#include "command.h"
#include "jobs.h"
#include "builtins.h"

/* Constants */
#define MAXCMDS 50      // Maximum number of commands in a pipeline
#define PROMPTLEN 2048  // Maximum length of shell prompt

/**
 * @brief Structure representing a shell command
 * 
 * Used to store information about a command and its associated job
 */
typedef struct shell_command {
    job* j;                 // Associated job
    char* shell_cmd[2];    // Command strings
} shell_command;

/* Command flags for pipeline processing */
#define OUTPIP  01    // Command pipes output to next command
#define INPIP   02    // Command receives input from previous command

/* External variables */
extern struct command cmds[];         // Array of parsed commands
extern pid_t shell;                   // Shell process ID
extern struct termios shell_modes;    // Terminal modes
extern char prompt[];                 // Shell prompt string
extern FILE* script;                  // Script file if running in script mode
extern int is_interactive;            // Flag indicating interactive mode
extern job* bg_jobs;                  // List of background jobs
extern int bg_jobs_number;            // Number of background jobs
extern int biggest_idx;               // Largest job index used

/* Core shell functions */

/**
 * @brief Parse a command line into a job structure
 * 
 * Breaks down a command line into commands, handling pipes and redirections
 * 
 * @param line Command line to parse
 * @return job* Pointer to created job structure, or NULL on error
 */
job* parseline(char *line);

/**
 * @brief Read a line of input from the user
 * 
 * Handles both interactive and script input modes
 * 
 * @param prompt Prompt to display in interactive mode
 * @param line Buffer to store the input line
 * @param sizeline Size of the line buffer
 * @return int Number of characters read, or -1 on EOF
 */
int promptline(char *prompt, char *line, int sizeline);

/**
 * @brief Initialize the shell prompt
 * 
 * Creates a prompt string in the format: username@hostname:path%
 * Handles home directory substitution with ~
 * 
 * @param prompt Buffer to store the formatted prompt
 */
void initialize_shell_prompt(char prompt[]);

/**
 * @brief Handle keyboard interrupt (Ctrl+C)
 * 
 * Displays a new prompt when the user presses Ctrl+C
 * 
 * @param sig Signal number (unused)
 */
void handle_keyboard_interrupt(int sig);

/* Signal handling functions */

/**
 * @brief Handle keyboard quit signal (Ctrl+\)
 * 
 * Displays a new prompt when the user presses Ctrl+\
 * 
 * @param sig Signal number (unused)
 */
void handle_keyboard_quit(int sig);

/**
 * @brief Handle keyboard suspend signal (Ctrl+Z)
 * 
 * Displays a new prompt when the user presses Ctrl+Z
 * 
 * @param sig Signal number (unused)
 */
void handle_keyboard_suspend(int sig);

#endif /* SHELL_H */