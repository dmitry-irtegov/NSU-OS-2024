#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

// Переменная для хранения количества сигналов SIGINT (обычно посылается с помощью Ctrl+C)
volatile sig_atomic_t signal_count = 0;

// Обработчик сигнала для SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    // Увеличиваем счетчик при каждом получении SIGINT
    signal_count;
    // Издаем звуковой сигнал
    printf("\a"); // \a — это символ для звукового сигнала (bell)
    fflush(stdout); // Очищаем буфер вывода, чтобы сигнал сразу был воспроизведен
}

// Обработчик сигнала для SIGQUIT (Ctrl+\)
void handle_sigquit(int sig) {
    // Выводим сообщение с количеством сигналов SIGINT
    printf("\nПрограмма завершена. Сигнал SIGINT прозвучал %d раз(а).\n", signal_count);
    // Завершаем программу
    exit(0);
}

int main() {
    // Устанавливаем обработчик сигнала для SIGINT
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Ошибка установки обработчика для SIGINT");
        return 1;
    }
    // Устанавливаем обработчик сигнала для SIGQUIT
    if (signal(SIGQUIT, handle_sigquit) == SIG_ERR) {
        perror("Ошибка установки обработчика для SIGQUIT");
        return 1;
    }

    // Бесконечный цикл
    while (1) {
        // Программа ничего не делает, просто ожидает сигналов
        pause(); // Ожидание любого сигнала
    }
}
