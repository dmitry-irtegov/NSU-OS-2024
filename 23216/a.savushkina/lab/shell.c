#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"
#include <stdlib.h>
#include <fcntl.h>

#define BACKGROUND 1
#define FOREGROUND 0

struct command cmds[MAXCMDS];

void main(int argc, char *argv[])
{
    int i;
    char line[1024]; /* allow large command lines */
    int ncmds;
    char prompt[50]; /* shell prompt */

    /* PLACE SIGNAL CODE HERE */

    // сигналы управляющего терминала 
    

    sprintf(prompt, "[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0)
    { /* until eof */
        if ((ncmds = parseline(line)) <= 0)
            continue; /* read next line */

        for (i = 0; i < ncmds; i++)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("Fork Failed");
                continue;
            }
            else if (pid == 0)
            {
                if (i == 0 && cmds[0].infile != NULL)
                {
                    int infile_fd = open(cmds[0].infile, O_RDONLY);
                    if (infile_fd < 0)
                    {
                        perror("open infile failed");
                        exit(EXIT_FAILURE);
                    }
                    dup2(infile_fd, 0);
                    close(infile_fd);
                }

                if (i == ncmds - 1)
                {
                    if (cmds[ncmds - 1].outfile != NULL)
                    {
                        int outfile_fd = open(cmds[ncmds - 1].outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (outfile_fd < 0)
                        {
                            perror("open outfile failed");
                            exit(EXIT_FAILURE);
                        }
                        dup2(outfile_fd, 1);
                        close(outfile_fd);
                    }
                    else if (cmds[ncmds - 1].appfile != NULL)
                    {
                        int appfile_fd = open(cmds[ncmds - 1].appfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if (appfile_fd < 0)
                        {
                            perror("open appfile failed");
                            exit(EXIT_FAILURE);
                        }
                        dup2(appfile_fd, 1);
                        close(appfile_fd);
                    }
                }

                if (execvp(cmds[i].cmdargs[0], cmds[i].cmdargs) < 0)
                {
                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }
            else
            {
                if (cmds[i].bgk == BACKGROUND)
                {
                    printf("Background process ID: %d\n", pid);
                    
                }
                else
                {
                    int status;
                    if (waitpid(pid, &status, 0) < 0)
                    {
                        perror("waitpid failed");
                        continue; 
                    }
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                    {
                        fprintf(stderr, "Command '%s' exited with status %d\n", cmds[i].cmdargs[0], WEXITSTATUS(status));
                    }
                }
            }
        }
    } /* close while */
}

/* PLACE SIGNAL CODE HERE */