#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

int main() {
    int server_fd, client_fd;  
    char *socket_path = "./socket";  //  путь до файла UNIX-сокета

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_un addr; // структура для хранения адреса. содержит поля: семейство адресов, путь до сокета
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path));

    if (access(addr.sun_path, F_OK) == 0) {
        printf("Предупреждение: сокет по данному пути %s уже существует. Удаляем его.\n", addr.sun_path);
        unlink(addr.sun_path);  // 
    }

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) { // связывание сокета с адресом. вернет ошибку, если адрес занят
        perror("bind error");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 1) == -1) { // второй параметр - backlog - макс. кол-во подключений, которые могут ждать (в очереди) обработки сервера
        perror("listen error");
        close(server_fd);
        unlink(socket_path);
        return 1;
    } // listen закончит работу как только подключится клиент

    printf("Сервер слушает...\n");

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept error");
        close(server_fd);
        unlink(socket_path);
        return 1;
    } // возвращает дескриптор клиента

    printf("Клиент подключился к серверу.\n");

    char buffer[BUFSIZ];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        // printf("Received a message: %s\n", buffer);

        if (write(client_fd, buffer, bytes_read) == -1) {
            perror("write error");
            break;
        }
    }
    

    if (bytes_read == 0) {
        printf("Клиент отключился от сервера.\n");
    } else if (bytes_read == -1) {
        perror("read error");
    }

    close(client_fd);
    close(server_fd);
    unlink(socket_path);
    printf("Сервер остановлен корректно.\n");
    return 0;
}

// Напишите две программы, взаимодействующих через Unix domain socket.
// Первый процесс (сервер) создает сокет и слушает на нем.  При
// присоединении клиента, сервер получает через соединение текст, 
// состоящий из символов верхнего и нижнего регистров, переводит его
// в верхний регистр и выводит в свой стандартный поток вывода, 
// аналогично задаче 25. Второй процесс (клиент) устанавливает 
// соединение с сервером и передает ему текст.  После разрыва 
// соединения клиентом, оба процесса завершаются.