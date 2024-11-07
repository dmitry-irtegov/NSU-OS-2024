#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int beep_counter = 0; // Обычный int для счётчика

// Функция для перевода числа в строку (асинхронно-безопасная)
void int_to_str(int num, char *buffer, size_t buffer_size) {
    int i = 0;
    
    // Если число равно 0, просто добавляем '0'
    if (num == 0) {
        if (buffer_size > 1) {
            buffer[i++] = '0';
            buffer[i] = '\0';
        }
        return;
    }

    // Записываем цифры числа в обратном порядке
    while (num > 0 && i < buffer_size - 1) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    // Добавляем терминальный нуль для завершения строки
    buffer[i] = '\0';

    // Разворачиваем строку, чтобы получить правильный порядок цифр
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

void signal_handler(int signal_number) {
    switch (signal_number) {
        case SIGINT:
            // При SIGINT издать звуковой сигнал и увеличить счетчик
            write(STDOUT_FILENO, "\a", 1);
            beep_counter++;  // Увеличиваем счетчик
            break;
        case SIGQUIT: {
            // Конвертируем beep_counter в строку
            char buffer[50];
            int_to_str(beep_counter, buffer, sizeof(buffer));
            
            // Выводим количество сигналов
            write(STDOUT_FILENO, buffer, sizeof(buffer));
            write(STDOUT_FILENO, "\n", 1);
            
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

