#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

volatile int beep_count = 0;

void handle_signal(int sig) {   
    switch (sig) {
        case SIGQUIT:
            write(1, "\nAmount of beeps: ", 18); // Фиксированная часть сообщения

            // Преобразуем beep_count в строку
            char count_str[40]; // Буфер для числа
            int len = 0;
            int temp = beep_count;
            
            switch (temp) {
                case 0: // Обрабатываем случай, когда beep_count равен 0
                    count_str[len++] = '0';
                    break;

                default: // Преобразуем число в строку, если оно больше 0
                    while (temp > 0) {
                        count_str[len++] = '0' + (temp % 10);
                        temp /= 10;
                    }

                    // Разворачиваем строку
                    for (int i = 0; i < len / 2; i++) {
                        char tmp = count_str[i];
                        count_str[i] = count_str[len - i - 1];
                        count_str[len - i - 1] = tmp;
                    }
                    break;
            }

            // Выводим число
            write(1, count_str, len);
            write(1, "\n", 1); // Переход на новую строку
            _exit(EXIT_SUCCESS); // Завершаем программу

        case SIGINT:
            write(1, "\a", 1); // Звуковой сигнал
            beep_count++; // Увеличиваем счётчик
            signal(SIGINT, handle_signal); // Устанавливаем обработчик повторно
            break;

        default:
            break;
    }
}

int main() {
    if (signal(SIGINT, handle_signal) == SIG_ERR) {
        perror("Fail to set SIGINT handler");
        return EXIT_FAILURE;
    }

    if (signal(SIGQUIT, handle_signal) == SIG_ERR) {
        perror("Fail to set SIGQUIT handler");
        return EXIT_FAILURE;
    }
    
    while (1) {
        pause();
    }

}