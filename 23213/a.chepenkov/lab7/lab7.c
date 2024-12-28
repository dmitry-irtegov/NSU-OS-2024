#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/select.h>

void print_line(const char* map, size_t* offsets, int line_num, int total_lines) {
    if (line_num < 1 || line_num > total_lines) {
        fprintf(stderr, "Wrong string number. Enter number from 1 to %d.\n", total_lines);
        return;
    }

    size_t start = offsets[line_num - 1];
    size_t end = (line_num == total_lines) ? strlen(map) : offsets[line_num];

    write(STDOUT_FILENO, map + start, end - start);
}

int main(int argc, char* argv[]) {

    setbuf(stdin, NULL);

    if (argc != 2) {
        fprintf(stderr, "Wrong arguments number\n");
        return 1;
    }

    const char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file\n");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }


    size_t file_size = st.st_size;

    char* map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("map failed");
        close(fd);
        return 1;
    }
    close(fd);

    size_t* offsets = malloc((file_size + 1) * sizeof(size_t));
    if (!offsets) {
        perror("malloc");
        munmap(map, file_size);
        return 1;
    }

    int line_count = 0;
    offsets[line_count++] = 0;
    for (size_t i = 0; i < file_size; i++) {
        if (map[i] == '\n') {
            offsets[line_count++] = i + 1;
        }
    }

    while (1) {
        printf("Enter string number (0 to exit):\n");

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int retval = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select");
            break;
        }
        else if (retval == 0) {
            printf("\nTime is over\n");
            for (int i = 0; i < line_count; i++) {
                print_line(map, offsets, i, line_count - 1);
            }
            break;
        }
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            int line_num;
            int scanf_res = scanf("%d", &line_num);

            if (scanf_res == EOF) {
                if (feof(stdin)) {
                    printf("EOF reached.\n");
                }
                else if (ferror(stdin)) {
                    perror("scanf");
                }
                break;
            }
            else if (scanf_res != 1) {
                printf("String must contain only numbers\n");
                scanf("%*s");
                continue;
            }

            if (line_num == 0) {
                break;
            }

            print_line(map, offsets, line_num, line_count - 1);
            printf("\n");
        }
    }
    free(offsets);
    munmap(map, file_size);
    return 0;
}
