void* print_lines(void* arg) {
    for (int i = 0; i < 10; i++) {
        puts("text");
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    // Создание нового потока
    pthread_create(&thread, NULL, print_lines, NULL);
    
    // Выполнение кода в главном потоке
    print_lines(NULL);
    
    // Ожидание завершения дочернего потока
    pthread_join(thread, NULL);
    
    return 0;
}
