#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Переменная для хранения количества сигналов SIGINT
volatile sig_atomic_t signal_count = 0;

// Простая функция для преобразования числа в строку
void int_to_string(int num, char* str) {
    int i = 0;

    // Определим количество цифр в числе
    for (int temp = num; temp != 0; temp /= 10) {
        i++;
    }

    str[i] = '\0'; // Завершение строки

    // Заполняем строку цифрами с конца
    while (i > 0) {
        str[--i] = '0' + num % 10;
        num /= 10;
    }
}

// Общий обработчик сигналов для SIGINT (Ctrl+C) и SIGQUIT (Ctrl+\)
void handle_signal(int sig) {
    switch (sig) {
        case SIGINT:
            signal_count++;
            write(STDOUT_FILENO, "\a", 1); // Звуковой сигнал
            break;
        default:
            // Завершаем программу и выводим количество SIGINT при получении SIGQUIT
            char buffer[100] = "\nПрограмма завершена. Количество SIGINT - ";
            char count_str[10] = {0}; // Для конвертации числа в строку
            int_to_string(signal_count, count_str); // Преобразуем счётчик в строку
            write(STDOUT_FILENO, buffer, 100);
            write(STDOUT_FILENO, count_str, 10);
            write(STDOUT_FILENO, "\n", 1);

            _exit(0); // Завершаем программу
    }
}

int main() {
    // Устанавливаем обработчик сигнала для SIGINT
    if (sigset(SIGINT, handle_signal) == SIG_ERR) {
        perror("Ошибка установки обработчика для SIGINT");
        return 1;
    }
    // Устанавливаем обработчик сигнала для SIGQUIT
    if (sigset(SIGQUIT, handle_signal) == SIG_ERR) {
        perror("Ошибка установки обработчика для SIGQUIT");
        return 1;
    }

    // Бесконечный цикл
    while (1) {
        pause(); // Ожидание любого сигнала
    }
}
