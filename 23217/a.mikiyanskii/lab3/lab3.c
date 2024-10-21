#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

void print_uids() {
    // ������ ��������� � ������������ ��������������� ������������
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

int main() {
    // ��� 1: ������ ������� ���������������
    printf("Before setuid:\n");
    print_uids();

    // ��� 2: ������� ������� ����
    FILE* file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
    }
    else {
        printf("File opened successfully!\n");
        fclose(file);
    }

    // ��� 3: ������� �������� � ����������� �������������� �����������
    if (setuid(getuid()) != 0) {
        perror("setuid failed");
        exit(1);
    }

    // ��� 4: ������ ��������������� ����� setuid
    printf("After setuid:\n");
    print_uids();

    // ��� 5: ������� ����� ������� ����
    file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file after setuid");
    }
    else {
        printf("File opened successfully after setuid!\n");
        fclose(file);
    }

    return 0;
}
