#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    struct termios oldt, newt;
    
     // Проверяем, что STDIN связан с терминалом
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: STDIN is not a terminal\n");
        exit(EXIT_FAILURE);
    }

    // Сохраняем текущие настройки терминала
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        perror("Error getting terminal settings");
        exit(EXIT_FAILURE);
    }

    newt = oldt;
    newt.c_lflag &= ~(ICANON); // Отключаем канонический режим и эхо
    newt.c_cc[VMIN] = 1; // Устанавливаем минимальный ввод в 1 символ

    // Применяем новые настройки терминала
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
        perror("Error changing terminal settings");
        exit(EXIT_FAILURE);
    }

    printf("Хотите ли вы продолжить?(y/n): ");
    fflush(stdout);

    char input;
    if (read(STDIN_FILENO, &input, 1) == -1) { // Проверяем успешное считывание одного символа
        perror("Error reading input");
        // Восстанавливаем старые настройки терминала перед выходом
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        exit(EXIT_FAILURE);
    }

    // Восстанавливаем старые настройки терминала
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) != 0) {
        perror("Error restoring terminal settings");
        exit(EXIT_FAILURE);
    }
    printf("\n");

    return 0;
}

