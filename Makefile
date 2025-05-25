CC = gcc
CFLAGS = -g
LDFLAGS = -lcurl -lncurses -lcjson -lssl -lcrypto

SRC = main.c utils.c oauth.c request.c
OBJ = $(SRC:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)