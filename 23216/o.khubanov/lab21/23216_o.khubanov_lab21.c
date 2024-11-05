#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int beep_counter = 0; // Обычный int для счётчика

void signal_handler(int signal_number) {
    switch (signal_number) {
        case SIGINT:
            // При SIGINT издать звуковой сигнал и увеличить счетчик
            write(STDOUT_FILENO, "\a", 1);
            beep_counter++;  // Увеличиваем счетчик
            break;
        case SIGQUIT: {
            // Конвертируем beep_counter в строку вручную
            int counter = beep_counter;
            char buffer[50];
            int i = 0;

            // Обратное заполнение buffer значением counter
            if (counter == 0) {
                buffer[i++] = '0';
            } else {
                while (counter > 0) {
                    buffer[i++] = '0' + (counter % 10);
                    counter /= 10;
                }
            }
            
            // Разворачиваем число в прямой порядок
            for (int j = 0; j < i / 2; j++) {
                char temp = buffer[j];
                buffer[j] = buffer[i - j - 1];
                buffer[i - j - 1] = temp;
            }

            buffer[i++] = '\n';  // Добавляем перенос строки
            write(STDOUT_FILENO, "Total number of beeps: ", 23);
            write(STDOUT_FILENO, buffer, i);
            _exit(EXIT_SUCCESS); // Используем _exit для немедленного завершения
        }
        default:
            // Игнорируем другие сигналы
            break;
    }
}
int main() {
    // Установка пользовательских обработчиков сигналов
    if (sigset(SIGINT, signal_handler) == SIG_ERR) {
        perror("Error setting SIGINT handler");
        return EXIT_FAILURE;
    }
    if (sigset(SIGQUIT, signal_handler) == SIG_ERR) {
        perror("Error setting SIGQUIT handler");
        return EXIT_FAILURE;
    }

    // Основной цикл программы: ожидание сигнала
    while (1) {
        pause(); // Приостановить выполнение до получения сигнала
    }

    return 0; // Программа сюда никогда не дойдет, но добавлено для завершенности
}

