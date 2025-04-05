#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd;
    struct termios tty, savtty;
    char ch;

    fd = fileno(stdin);
    if (isatty(fd) == 0) {
        fprintf(stderr, "stdin not terminal\n");
        exit(1);
    }

    //void perror(const char *s) 
    //печатает сообщение описывающее ошибку на основе номера последней ошибки в errno.
    //А именно, сначала печатается s, затем двоеточие и пробел и затем собственно сообщение об ошибке
    //и заканчивается все символом переноса строки.
    //Поэтому s не может заканчиватся символом переноса строки, иначе вывод будет кривой.
    if (tcgetattr(fd, &tty) != 0) {
        perror("can't get attributes");
        exit(1);
    }

    savtty = tty;
    tty.c_lflag &= ~(ICANON|ISIG);
    tty.c_cc[VMIN] = 1;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("can't set attributes");
        exit(1);
    }

    //setbuf(stdout, (char*)NULL);
    //Вызов этой функции запрещает локальную буферизацию, поэтому printf выводит строки сразу, а не после выполнения всей программы.
    //При включенной буферизации printf выводит строку на экран после завершения программы, либо после переноса строки.
    //Далее в коде все выводимые строки заканчиваются переносом строки(изначально я его добавлял туда для лучшей читаемости),
    //Так что в вызове setbuf нет необходимости.
    
    printf("What's the third letter in the word TERMINAL?\n");
    read(fd, &ch, 1);
    printf("\n");
    if (ch == 'R') {
        printf("Correct!\n");
    } else {
        printf("Wrong.\n");
    }

    if(tcsetattr(fd, TCSANOW, &savtty) != 0) {
        perror("can't set old attributes\n");
        exit(1);
    }
}
