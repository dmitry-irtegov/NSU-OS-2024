#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "builtins.h"
#include "shell.h"
#include "jobs.h"

/**
 * @brief Store previous directory
 */
static char prev_dir[PROMPTLEN] = "";

/**
 * @brief Display help information about built-in commands
 */
void help(void) {
    printf("Built-in commands:\n");
    printf("  cd [dir]     Change directory. Without arguments changes to home directory.\n");
    printf("               cd -  Changes to previous directory.\n");
    printf("  jobs         List all background jobs.\n");
    printf("  fg [%%job]    Bring job to foreground. Without arguments brings last job.\n");
    printf("  bg [%%job]    Continue stopped job in background.\n");
    printf("  exit         Exit the shell.\n");
    printf("  help         Display this help message.\n");
    printf("\nOther useful commands (provided by system):\n");
    printf("  pwd          Print current working directory.\n");
    printf("  echo [args]  Display arguments.\n");
    printf("  ls [-la]     List directory contents.\n");
    printf("  ps [-f]      Report process status.\n");
    printf("  and many more...\n");
}

/**
 * @brief Expand path starting with tilde (~)
 * 
 * @param path Path to expand
 * @param expanded_path Buffer to store expanded path
 * @return char* Pointer to expanded path or NULL on error
 */
static char* expand_tilde(const char* path, char* expanded_path) {
    char *home;
    if (path[1] == '/' || path[1] == '\0') {
        home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return NULL;
        }
        if (path[1] == '\0') {
            return home;
        }
        snprintf(expanded_path, PROMPTLEN, "%s%s", home, path + 1);
        return expanded_path;
    }
    
    // Handle ~username
    const char *username = path + 1;  
    const char *slash = strchr(username, '/');
    char username_copy[PROMPTLEN];
    
    if (slash != NULL) {
        size_t len = slash - username;
        strncpy(username_copy, username, len);
        username_copy[len] = '\0';
    } else {
        strncpy(username_copy, username, PROMPTLEN-1);
        username_copy[PROMPTLEN-1] = '\0';
    }

    struct passwd *pw = getpwnam(username_copy);
    if (pw == NULL) {
        fprintf(stderr, "cd: unknown user %s\n", username_copy);
        return NULL;
    }

    if (slash != NULL) {
        snprintf(expanded_path, PROMPTLEN, "%s%s", pw->pw_dir, slash);
    } else {
        strncpy(expanded_path, pw->pw_dir, PROMPTLEN-1);
        expanded_path[PROMPTLEN-1] = '\0';
    }
    return expanded_path;
}

/**
 * @brief Expand environment variable in path
 * 
 * @param var Environment variable to expand (including $ symbol)
 * @return char* Pointer to expanded value or NULL on error
 */
static char* expand_env_var(const char* var) {
    char *env_name = (char*)var + 1;  // Skip $
    char *env_value = getenv(env_name);
    if (env_value == NULL) {
        fprintf(stderr, "cd: %s not set\n", var);
        return NULL;
    }
    
    // For $PWD, check that directory exists and is accessible
    if (strcmp(env_name, "PWD") == 0) {
        struct stat st;
        if (stat(env_value, &st) == -1) {
            if (errno == ENOENT) {
                fprintf(stderr, "%s: No such file or directory\n", var);
            } else if (errno == EACCES) {
                fprintf(stderr, "cd: permission denied: %s\n", var);
            } else {
                perror("cd");
            }
            return NULL;
        }
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s: Not a directory\n", var);
            return NULL;
        }
    }
    return env_value;
}

/**
 * @brief Change current directory to target
 * 
 * @param target Target directory path
 * @param arg Original argument (for error messages)
 * @return int 0 on success, 1 on error
 */
static int change_to_directory(const char* target, const char* arg) {
    if (chdir(target) == -1) {
        switch(errno) {
            case EACCES:
                fprintf(stderr, "cd: permission denied: %s\n", arg);
                break;
            case ENOENT:
                fprintf(stderr, "%s: No such file or directory\n", arg);
                break;
            case ENOTDIR:
                fprintf(stderr, "%s: Not a directory\n", arg);
                break;
            default:
                perror("cd");
        }
        return 1;
    }
    return 0;
}

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
int cd(char **args, char *prompt) {
    char current_dir[PROMPTLEN];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return 1;
    }

    if (args[1] != NULL && args[2] != NULL) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }

    char *target_dir = NULL;
    static char expanded_path[PROMPTLEN];
    
    // Handle cd without arguments or cd ~
    if (args[1] == NULL || strcmp(args[1], "~") == 0) {
        target_dir = getenv("HOME");
        if (target_dir == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    // Handle cd -
    else if (strcmp(args[1], "-") == 0) {
        if (prev_dir[0] == '\0') {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return 1;
        }
        target_dir = prev_dir;
        printf("%s\n", target_dir);
    }
    // Handle tilde expansion
    else if (args[1][0] == '~') {
        target_dir = expand_tilde(args[1], expanded_path);
        if (target_dir == NULL) return 1;
    }
    // Handle environment variables
    else if (args[1][0] == '$') {
        target_dir = expand_env_var(args[1]);
        if (target_dir == NULL) return 1;
    }
    // Regular path
    else {
        target_dir = args[1];
    }

    if (change_to_directory(target_dir, args[1]) != 0) {
        return 1;
    }

    // Update prev_dir and prompt
    strcpy(prev_dir, current_dir);
    initialize_shell_prompt(prompt);
    return 0;
}

// Enumeration for built-in commands
typedef enum {
    CMD_NONE = 0,
    CMD_HELP,
    CMD_CD,
    CMD_JOBS,
    CMD_FG,
    CMD_BG,
    CMD_EXIT
} builtin_cmd;

/**
 * @brief Determine if command is built-in
 * 
 * @param cmd Command to check
 * @return builtin_cmd Command type or CMD_NONE if not built-in
 */
static builtin_cmd get_builtin_command(const char* cmd) {
    if (!cmd) return CMD_NONE;
    
    if (!strcmp(cmd, "help")) return CMD_HELP;
    if (!strcmp(cmd, "cd")) return CMD_CD;
    if (!strcmp(cmd, "jobs")) return CMD_JOBS;
    if (!strcmp(cmd, "fg")) return CMD_FG;
    if (!strcmp(cmd, "bg")) return CMD_BG;
    if (!strcmp(cmd, "exit")) return CMD_EXIT;
    
    return CMD_NONE;
}

/**
 * @brief Execute built-in command
 * 
 * @param cmd Built-in command to execute
 * @param args Command arguments
 * @param prompt Shell prompt
 * @return int 1 on success, -1 on error, 0 if not built-in
 */
static int execute_builtin(builtin_cmd cmd, char **args, char *prompt) {
    switch(cmd) {
        case CMD_HELP:
            help();
            return 1;
            
        case CMD_CD: {
            int result = cd(args, prompt);
            return result == 0 ? 1 : -1;  // Return -1 on error
        }
            
        case CMD_JOBS:
            monitor_jobs();
            jobs();
            return 1;
            
        case CMD_FG:
            fg(args);
            return 1;
            
        case CMD_BG:
            bg(args);
            return 1;
            
        case CMD_EXIT:
            exit(0);  // Simply exit without additional output
            
        default:
            return 0;
    }
}

/**
 * @brief Check and execute built-in command
 * 
 * @param args Command arguments
 * @param prompt Shell prompt
 * @return int 1 if command was built-in and executed successfully,
 *         -1 if built-in command failed,
 *          0 if command is not built-in
 */
int is_builtin(char **args, char *prompt) {
    if (!args[0]) return 0;

    builtin_cmd cmd = get_builtin_command(args[0]);
    return execute_builtin(cmd, args, prompt);
}
