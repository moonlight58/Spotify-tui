CC = gcc
CFLAGS = -g -Iinclude -MMD -MP
LDFLAGS = -lcurl -lncurses -lcjson -lssl -lcrypto

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
DEP = $(OBJ:.o=.d)
TARGET ?= main

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build $(TARGET)

-include $(DEP)