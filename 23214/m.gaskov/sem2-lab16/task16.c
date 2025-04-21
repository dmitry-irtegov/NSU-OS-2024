#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void print_error(int result, const char *msg, bool should_exit, bool should_unlink) {
    if (result == -1) {
        perror(msg);
        if (should_unlink) {
            sem_unlink("/sem_parent");
            sem_unlink("/sem_child");
        }
        if (should_exit) {
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    int i;
    sem_t *sem_parent, *sem_child;

    sem_parent = sem_open("/sem_parent", O_CREAT | O_EXCL, 0600, 1);
    if (sem_parent == SEM_FAILED) {
        perror("sem_open failed for sem_parent");
        exit(EXIT_FAILURE);
    }
    sem_child = sem_open("/sem_child", O_CREAT | O_EXCL, 0600, 0);
    if (sem_child == SEM_FAILED) {
        perror("sem_open failed for sem_child");
        sem_close(sem_parent);
        sem_unlink("/sem_parent");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    print_error(pid = fork(), "Fork failed", 1, 1);

    if (pid == 0) {
        for (i = 0; i < 10; i++) {
            print_error(sem_wait(sem_child), "Child: sem_wait failed", 1, 0);
            printf("Child line: %d\n", i + 1);
            print_error(sem_post(sem_parent), "Child: sem_post failed", 1, 0);
        }
        sem_close(sem_parent);
        sem_close(sem_child);
        exit(EXIT_SUCCESS);
    } else {
        for (i = 0; i < 10; i++) {
            print_error(sem_wait(sem_parent), "Parent: sem_wait failed", 1, 1);
            printf("Parent line: %d\n", i + 1);
            print_error(sem_post(sem_child), "Parent: sem_post failed", 1, 1);
        }
        print_error(wait(NULL), "wait failed", 1, 1);
        sem_close(sem_parent);
        sem_close(sem_child);
        sem_unlink("/sem_parent");
        sem_unlink("/sem_child");
        exit(EXIT_SUCCESS);
    }
}
