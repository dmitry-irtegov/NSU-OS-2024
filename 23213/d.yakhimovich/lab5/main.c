#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 512
#define LINE_CAP 10

typedef struct
{
    off_t offset;
    size_t length;
} Line;

Line *alloc_lines(int initial_capacity);
void realloc_lines(Line **lines, int *capacity);
int read_file(int fd, Line **lines, int *line_count);
int print_line(int fd, Line *lines, int line_number);
void print_line_table(Line *lines, int line_count);
int handle_user_input(int fd, Line *lines, int line_count);
int open_file(char *filename);
void close_file(int fd);
void clear_input();

int main(int argc, char *argv[])
{
    int debug_mode = 0, filename_argv = 1;
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "usage: %s [-d] [file]\n", argv[0]);
        exit(1);
    }

    if (argc == 3)
    {
        debug_mode = 1;
        if (strcmp(argv[1], "-d") == 0)
        {
            filename_argv = 2;
        }
        else if (strcmp(argv[2], "-d") == 0)
        {
            filename_argv = 1;
        }
        else
        {
            fprintf(stderr, "usage: %s [-d] [file]\n", argv[0]);
            exit(1);
        }
    }

    int fd = open_file(argv[filename_argv]);
    Line *lines = NULL;
    int line_count = 0;

    if (read_file(fd, &lines, &line_count) == -1)
    {
        free(lines);
        close_file(fd);
        exit(1);
    }

    if (debug_mode)
    {
        print_line_table(lines, line_count);
    }

    if (handle_user_input(fd, lines, line_count) == -1)
    {
        free(lines);
        close_file(fd);
        exit(1);
    }

    free(lines);
    close_file(fd);
    return 0;
}

Line *alloc_lines(int initial_capacity)
{
    Line *lines = (Line *)malloc(initial_capacity * sizeof(Line));
    if (lines == NULL)
    {
        perror("malloc");
        exit(1);
    }
    return lines;
}

void realloc_lines(Line **lines, int *capacity)
{
    *capacity *= 2;
    *lines = (Line *)realloc(*lines, *capacity * sizeof(Line));
    if (*lines == NULL)
    {
        perror("realloc");
        exit(1);
    }
}

int read_file(int fd, Line **lines, int *line_count)
{
    char buffer[BUFFER_SIZE];
    off_t offset = 0;
    ssize_t bytes_read;
    int i, count = 0, capacity = LINE_CAP;

    *lines = alloc_lines(capacity);
    (*lines)[count].offset = 0;
    count++;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        for (i = 0; i < bytes_read; i++)
        {
            if (buffer[i] == '\n')
            {
                (*lines)[count - 1].length = offset + i + 1 - (*lines)[count - 1].offset;
                if (count >= capacity)
                {
                    realloc_lines(lines, &capacity);
                }
                (*lines)[count].offset = offset + i + 1;
                count++;
            }
        }
        offset += bytes_read;
    }

    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    if (offset > 0 && buffer[bytes_read - 1] != '\n')
    {
        (*lines)[count - 1].length = offset - (*lines)[count - 1].offset;
        count++;
    }

    *line_count = count - 1;
    return 0;
}

int print_line(int fd, Line *lines, int line_number)
{
    if (line_number < 0)
    {
        printf("Invalid line number!\n");
        return 1;
    }

    lseek(fd, lines[line_number].offset, SEEK_SET);

    ssize_t bytes_read;
    char *buffer = (char *)malloc(lines[line_number].length + 1);
    if (buffer == NULL)
    {
        perror("malloc");
        return -1;
    }

    bytes_read = read(fd, buffer, lines[line_number].length);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }
    buffer[lines[line_number].length] = '\0';

    printf("%s", buffer);
    free(buffer);
    return 0;
}

void print_line_table(Line *lines, int line_count)
{
    for (int i = 0; i < line_count; i++)
    {
        printf("Line %d: offset = %ld, len = %zu\n", i + 1, lines[i].offset, lines[i].length);
    }
}

int handle_user_input(int fd, Line *lines, int line_count)
{
    int line_number, scan_res;
    char line_end;

    while (1)
    {
        printf("Enter line number (0 for exit): ");
        scan_res = scanf("%d", &line_number);
        scanf("%c", &line_end);

        if (scan_res != 1 || line_end != '\n')
        {
            printf("Invalid input!\n");
            clear_input();
            continue;
        }

        if (line_number == 0)
        {
            break;
        }
        else if (line_number > 0 && line_number <= line_count)
        {
            if (print_line(fd, lines, line_number - 1) != 0)
            {
                return -1;
            }
            if (line_count == line_number)
            {
                printf("\n");
            }
        }
        else
        {
            printf("Line number out of range!\n");
        }
    }
    return 0;
}

int open_file(char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }
    return fd;
}

void close_file(int fd)
{
    if (close(fd) == -1)
    {
        perror("close");
        exit(1);
    }
}

void clear_input()
{
    char trash;
    do
    {
        trash = getchar();
    } while (trash != '\n');
}