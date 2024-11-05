#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int beep_counter = 0; // Статическая переменная для подсчета звуковых сигналов

void signal_handler(int signal_number) {
    switch (signal_number) {
        case SIGINT:
            // При SIGINT издать звуковой сигнал и увеличить счетчик
            write(STDOUT_FILENO, "\a", 1);
            beep_counter++;
            break;
        case SIGQUIT:
            // При SIGQUIT вывести количество звуковых сигналов и завершить программу
            dprintf(STDOUT_FILENO, "\nTotal number of beeps: %d\n", beep_counter);
            _exit(EXIT_SUCCESS);
        default:
            // Игнорировать все остальные сигналы
            break;
    }
}

int main() {
    // Установка пользовательских обработчиков сигналов
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Error setting SIGINT handler");
        return EXIT_FAILURE;
    }
    if (signal(SIGQUIT, signal_handler) == SIG_ERR) {
        perror("Error setting SIGQUIT handler");
        return EXIT_FAILURE;
    }

    // Основной цикл программы: ожидание сигнала
    while (1) {
        pause(); // Приостановить выполнение до получения сигнала
    }

    return 0; // Программа сюда никогда не дойдет, но добавлено для завершенности
}
