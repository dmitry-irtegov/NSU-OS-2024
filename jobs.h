/**
 * @file jobs.h
 * @brief Job control header file for shell implementation
 *
 * This file defines the structures and functions needed for job control
 * in the shell, including process and job management, background and
 * foreground execution, and job status tracking.
 */

#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include <termios.h>
#include "command.h"

/**
 * @brief Process structure representing a single command in a pipeline
 *
 * Contains information about a process including its PID, command arguments,
 * status, and file descriptors for I/O redirection.
 */
typedef struct process {
    struct process* prev;         /* Previous process in pipeline */
    struct process* next;         /* Next process in pipeline */
    pid_t p_pid;                  /* Process ID */
    pid_t g_pid;                  /* Process group ID */
    struct command argv;          /* Command and its arguments */
    int status;                  /* 0 - terminated, 1 - active, 2 - stopped */
    int fildes[2];              /* Pipe file descriptors */
    int prev_fildes[2];         /* Previous pipe file descriptors */
    int bg;                      /* Background execution flag */
    char* infile;               /* Input redirection file */
    char* outfile;              /* Output redirection file */
    char* appfile;              /* Append redirection file */
    char* errfile;              /* Error redirection file */
} process;

/**
 * @brief Job structure representing a pipeline of processes
 *
 * Contains information about a job including its processes, status,
 * and job control information.
 */
typedef struct job {
    int idx;                    /* Job index */
    struct job* prev;           /* Previous job in list */
    struct job* next;           /* Next job in list */
    pid_t job_pgid;             /* Process group ID */
    process* processes;          /* List of processes in job */
    int status;                /* 0 - terminated, 1 - active, 2 - stopped */
    int fildes[2];              /* Pipe file descriptors */
    int bg;                    /* Background execution flag */
    char* job_name;            /* Job name (command line) */
    char cmdflag;              /* Command flag */
    char* infile;              /* Input redirection file */
    char* outfile;             /* Output redirection file */
    char* appfile;             /* Append redirection file */
    char* errfile;             /* Error redirection file */
} job;

/* Global variables */
extern job* bg_jobs;            /* List of background jobs */
extern int bg_jobs_number;      /* Number of background jobs */
extern int biggest_idx;         /* Largest job index */
extern int is_interactive;      /* Interactive shell flag */

/**
 * @brief Start execution of a process
 * 
 * Sets up the process environment, handles I/O redirections,
 * and executes the command.
 *
 * @param p Process to start
 */
void start_process(process* p);

/**
 * @brief Start execution of a job
 * 
 * Initializes and starts all processes in the job pipeline,
 * handling both foreground and background execution.
 *
 * @param j Job to start
 */
void start_job(job* j);

/**
 * @brief Wait for job completion and handle its state
 * 
 * Waits for all processes in a job to complete and updates
 * job status accordingly.
 *
 * @param j Job to wait for
 */
void wait_and_handle_job_completion(job* j);

/**
 * @brief Bring a background job to foreground
 * 
 * Continues execution of a background job in the foreground.
 *
 * @param args Command arguments (can be NULL or job index)
 */
void fg(char **args);

/**
 * @brief Put a stopped job in background
 * 
 * Continues execution of a stopped job in the background.
 *
 * @param args Command arguments (can be NULL or job index)
 */
void bg(char **args);

/**
 * @brief Display information about all background jobs
 * 
 * Lists all background jobs with their status, index,
 * and command line.
 */
void jobs();

/**
 * @brief Free a process structure and its arguments
 * 
 * Deallocates all memory associated with a process structure,
 * including its command arguments.
 *
 * @param p Process to free
 */
void cleanup_process_and_arguments(process* p);

/**
 * @brief Free a job structure
 * 
 * Deallocates all memory associated with a job structure,
 * including its processes and command arguments.
 *
 * @param j Job to free
 */
void free_job(job* j);

/**
 * @brief Monitor background jobs and handle their state changes
 * 
 * Checks status of background jobs and updates their state,
 * removing completed jobs and handling stopped jobs.
 */
void monitor_jobs();

/**
 * @brief Find a job by process group ID
 * 
 * Searches through the job list to find a job containing
 * a process with the specified process group ID.
 *
 * @param gpid Process group ID to find
 * @return Job containing the process or NULL if not found
 */
job* find_job(pid_t gpid);

/**
 * @brief Remove a job from jobs list
 * 
 * Removes a job from the background jobs list and updates
 * job indices accordingly.
 *
 * @param j Job to remove
 */
void remove_job(job* j);

/**
 * @brief Build a job structure from parsed commands
 * 
 * Creates and initializes a job structure with processes based on the
 * provided command array. Sets up process pipelines and I/O redirections.
 *
 * @param j Job structure to build
 * @param cmds Array of parsed commands
 * @param ncmds Number of commands to process
 * @param curr_idx Starting index in command array
 */
void build_job(job* j, struct command* cmds, int ncmds, int curr_idx);

#endif /* JOBS_H */
