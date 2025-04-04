#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "shell.h"


int parseline(char *line);
void test_single_command_no_args()
{
    char line[] = "command";
    int result = parseline(line);
    assert(result == 1);
    assert(strcmp(cmds[0].cmdargs[0], "command") == 0);
    assert(cmds[0].cmdargs[1] == NULL);
    assert(cmds[0].cmdflag == 0);
    assert(cmds[0].infile == NULL);
    assert(cmds[0].outfile == NULL);
    assert(cmds[0].appfile == NULL);
    assert(cmds[0].bgk == 0);
}

void test_semicolon()
{
    char input[] = "ls -l ; echo hello ; pwd";
    int result = parseline(input);

    assert(result == 3); 
    assert(strcmp(cmds[0].cmdargs[0], "ls") == 0);
    assert(strcmp(cmds[0].cmdargs[1], "-l") == 0);
    assert(cmds[0].cmdargs[2] == NULL);

    assert(strcmp(cmds[1].cmdargs[0], "echo") == 0);
    assert(strcmp(cmds[1].cmdargs[1], "hello") == 0);
    assert(cmds[1].cmdargs[2] == NULL);

    assert(strcmp(cmds[2].cmdargs[0], "pwd") == 0);
    assert(cmds[2].cmdargs[1] == NULL);

    assert(cmds[0].bgk == 0);
}
void test_input_redirection()
{
    char input[] = "cat < input.txt";
    int result = parseline(input);

    assert(result == 1);
    assert(strcmp(cmds[0].cmdargs[0], "cat") == 0);
    assert(cmds[0].cmdargs[1] == NULL);
    assert(strcmp(cmds[0].infile, "input.txt") == 0);
    assert(cmds[0].outfile == NULL);
    assert(cmds[0].appfile == NULL);
    assert(cmds[0].cmdflag == 0);
    assert(cmds[0].bgk == 0);

    printf("Test input redirection passed\n");
}
void test_output_redirection_and_appending()
{
    char line[] = "echo Hello > output.txt";
    int result = parseline(line);

    assert(result == 1);
    assert(strcmp(cmds[0].cmdargs[0], "echo") == 0);
    assert(strcmp(cmds[0].cmdargs[1], "Hello") == 0);
    assert(cmds[0].cmdargs[2] == NULL);
    assert(strcmp(cmds[0].outfile, "output.txt") == 0);
    assert(cmds[0].appfile == NULL);
    char input[] = "ls -l ; echo hello ; pwd";

    char line2[] = "echo World >> output.txt";
    result = parseline(line2);

    assert(result == 1);
    assert(strcmp(cmds[0].cmdargs[0], "echo") == 0);
    assert(strcmp(cmds[0].cmdargs[1], "World") == 0);
    assert(cmds[0].cmdargs[2] == NULL);
    assert(cmds[0].outfile == NULL);
    assert(strcmp(cmds[0].appfile, "output.txt") == 0);

    printf("Test output redirection and appending passed.\n");
}

void test_output_redirection_()
{
    char input[] = "ls -l ; echo hello ; pwd";
    int result = parseline(input);
    assert(result == 3);
    assert(strcmp(cmds[0].cmdargs[0], "ls") == 0);
    printf("\nresult %d\n", result);
    assert(strcmp(cmds[0].cmdargs[1], "-l") == 0);
    printf("\nresult %d\n", result);
    assert(cmds[0].cmdargs[2] == NULL);
    printf("\nresult %d\n", result);

    assert(strcmp(cmds[1].cmdargs[0], "echo") == 0);
    assert(strcmp(cmds[1].cmdargs[1], "hello") == 0);
    assert(cmds[1].cmdargs[2] == NULL);

    assert(strcmp(cmds[2].cmdargs[0], "pwd") == 0);
    assert(cmds[2].cmdargs[1] == NULL);

    assert(cmds[0].bgk == 0);
}

void test_parseline_invalid_input()
{
    char input[] = "command |";
    int result = parseline(input);
    assert(result == -1);
}
void test_parseline_background_flag()
{
    char line[] = "command &";
    int result = parseline(line);
    printf("%d\n", result);

    assert(result == 1);
    assert(cmds[0].bgk == 1);
    assert(strcmp(cmds[0].cmdargs[0], "command") == 0);
    assert(cmds[0].cmdargs[1] == NULL);
}

void test_piping_between_command()
{
    char line[] = "ls -l | grep .txt | wc -l";
    int result = parseline(line);

    assert(result == 3); 

    assert(strcmp(cmds[0].cmdargs[0], "ls") == 0);
    assert(strcmp(cmds[0].cmdargs[1], "-l") == 0);
    assert(cmds[0].cmdargs[2] == NULL);
    assert(cmds[0].cmdflag & OUTPIP);

    assert(strcmp(cmds[1].cmdargs[0], "grep") == 0);
    assert(strcmp(cmds[1].cmdargs[1], ".txt") == 0);
    assert(cmds[1].cmdargs[2] == NULL);
    assert(cmds[1].cmdflag & INPIP);
    assert(cmds[1].cmdflag & OUTPIP);

    assert(strcmp(cmds[2].cmdargs[0], "wc") == 0);
    assert(strcmp(cmds[2].cmdargs[1], "-l") == 0);
    assert(cmds[2].cmdargs[2] == NULL);
    assert(cmds[2].cmdflag & INPIP);

    assert(cmds[0].bgk == 0);
}

void test_max_commands()
{
    char line[1000] = "";
    for (int i = 0; i < MAXCMDS - 1; i++)
    {
        strcat(line, "command");
        strcat(line, " | ");
    }
    strcat(line, "lastcommand");

    int result = parseline(line);

    assert(result == MAXCMDS);
    for (int i = 0; i < MAXCMDS; i++)
    {
        if (i < MAXCMDS - 1)
        {
            assert(strcmp(cmds[i].cmdargs[0], "command") == 0);
            assert(cmds[i].cmdflag & OUTPIP);
        }
        else
        {
            assert(strcmp(cmds[i].cmdargs[0], "lastcommand") == 0);
        }
        if (i > 0)
        {
            assert(cmds[i].cmdflag & INPIP);
        }
    }
    assert(cmds[0].bgk == 0);

    printf("Test maximum number of commands passed\n");
}

void test_no_valid_command()
{
    char line[] = "   \t\n";
    int result = parseline(line);

    assert(result == -1);
    assert(cmds[0].cmdargs[0] == NULL);
    assert(cmds[0].cmdflag == 0);
    assert(cmds[0].infile == NULL);
    assert(cmds[0].outfile == NULL);
    assert(cmds[0].appfile == NULL);
    assert(cmds[0].bgk == 0);

    printf("Test no valid command passed\n");
}

int main()
{
    printf("Test parseline with");
    test_single_command_no_args();
    test_semicolon();
    test_input_redirection();
    test_output_redirection_and_appending();
    test_output_redirection_();
    test_parseline_invalid_input();
    test_parseline_background_flag();
    test_piping_between_command();
    test_max_commands();
    test_no_valid_command();

    printf("All tests passed.\n");
    return 0;
}
