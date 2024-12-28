#ifndef BUILTINS_H
#define BUILTINS_H

/**
 * @file builtins.h
 * @brief Built-in shell commands implementation
 *
 * This header defines the interface for built-in shell commands
 * such as cd, help, jobs, fg, bg, and exit.
 */

#include "command.h"

/**
 * @brief Display help information about built-in commands
 */
void help(void);

/**
 * @brief Change current working directory
 * 
 * Handles special cases:
 * - No arguments: change to HOME
 * - '-': change to previous directory
 * - '~': expand to HOME or user's home directory
 * - '$VAR': expand environment variable
 * 
 * @param args Command arguments (args[0] is "cd")
 * @param prompt Shell prompt to update
 * @return int 0 on success, 1 on error
 */
int cd(char **args, char *prompt);

/**
 * @brief Check and execute built-in command
 * 
 * @param args Command arguments
 * @param prompt Shell prompt
 * @return int 1 if command was built-in and executed successfully,
 *         -1 if built-in command failed,
 *          0 if command is not built-in
 */
int is_builtin(char **args, char *prompt);

#endif /* BUILTINS_H */
