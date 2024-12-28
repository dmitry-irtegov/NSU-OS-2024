/**
 * @file promptline.c
 * @brief Shell prompt and command line input implementation
 *
 * This file implements the shell's prompt and command line input functionality including:
 * - Displaying the shell prompt
 * - Reading and handling user input
 * - Command line history management
 * - Signal handling during input
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "shell.h"

/**
 * @brief Reads a line of input from the user, handling continuation lines and EOF
 * 
 * Displays the shell prompt and reads a line of input from the user.
 * Handles:
 * - Line continuation
 * - Input buffer management
 * - Error conditions
 *
 * @param prompt The shell prompt to display
 * @param line The buffer to store the input line
 * @param sizline The size of the input buffer
 * @return The length of the input line on success, -1 on EOF or error
 */
int promptline(char *prompt, char *line, int sizline) {
    int n = 0;
    FILE* desc;
    int len;
    
    if (script) {
        desc = script;
    } else {
        desc = stdin;
    }
    
    printf("%s", prompt);
    fflush(stdout);
    
    while ((fgets(line + n, sizline - n, desc))) {
        len = strlen(line + n);
        n += len;
        
        // Check for newline
        if (len > 0 && line[n-1] == '\n') {
            // For the exit command, leave \n in the buffer
            if (n >= 4 && strncmp(line, "exit", 4) == 0) {
                char *p = line + 4;
                while (*p == ' ' || *p == '\t') p++;
                if (*p == '\n') {
                    return n;
                }
            }
            
            n--; // Return to the character before \n
            line[n] = '\0';
            
            // Check for backslash
            if (n > 0 && line[n-1] == '\\') {
                n--; // Remove backslash
                printf("> "); // Prompt for line continuation
                fflush(stdout);
                continue; // Read the next line
            }
            break;
        }
        
        if (n >= sizline - 1) {
            // Buffer is full
            line[n] = '\0';
            break;
        }
    }
    
    if (n == 0 && feof(desc)) {
        line[0] = '\0';
        return -1;
    }
    
    return n;
}