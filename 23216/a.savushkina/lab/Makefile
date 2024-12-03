# Имя исполняемого файла
TARGET = shell
TEST_TARGET = parseline_test

# Исходные файлы
SRCS = shell.c parseline.c promptline.c
TEST_SRCS = parseline.test.c parseline.c

# Компилятор
CC = gcc

# Флаги компилятора
CFLAGS = -Wall -Wextra -g

# Объектные файлы
OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Правило по умолчанию
all: $(TARGET)

# Правило для сборки основного исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Правило для сборки тестового исполняемого файла
test: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $^

# Правило для компиляции .c файлов в .o файлы
%.o: %.c shell.h
	$(CC) $(CFLAGS) -c $<

# Правило для очистки
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJS) $(TEST_OBJS)

# Дополнительные цели
.PHONY: all test clean
