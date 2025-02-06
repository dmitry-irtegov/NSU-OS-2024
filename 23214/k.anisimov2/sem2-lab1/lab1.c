#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Функция, выполняемая в новом потоке
void *thread_body(void *param)
{
    for (int i = 1; i < 11; ++i)
    {
        printf("Newly created thread: line %d\n", i);
        usleep(100000); // задержка для чередования вывода
    }
    return NULL;
}

int main()
{
    pthread_t thread;

    // Создание нового потока с атрибутами по умолчанию
    if (pthread_create(&thread, NULL, thread_body, NULL) != 0)
    {
        perror("ERROR creating thread");
        exit(EXIT_FAILURE);
    }

    // Основной поток
    for (int i = 1; i < 11; ++i)
    {
        printf("Parent thread: line %d\n", i);
        usleep(100000);
    }

    // Ожидание завершения потока
    pthread_join(thread, NULL);
    return EXIT_SUCCESS;
}