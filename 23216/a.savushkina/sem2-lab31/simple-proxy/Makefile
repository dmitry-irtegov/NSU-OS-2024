CC = gcc
CFLAGS = -Wall -Wextra -I./src
SRC = src/main.c src/cache.c src/proxy.c src/utils.o
OBJ = $(SRC:.c=.o)
TARGET = simple-proxy

all: $(TARGET)

$(TARGET): $(OBJ)
        $(CC) -o $@ $(OBJ) -lnsl -lsocket

%.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

clean:
        rm -f $(OBJ) $(TARGET)

.PHONY: all clean
