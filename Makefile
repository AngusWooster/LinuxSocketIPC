CC:=gcc
INC_PATH = ./inc
CFLAG +=-g -I$(INC_PATH)
TARGET = server client

.PHONY: all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@

server: srv.o
	$(CC) -o $@ $^

client: client.o
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm *.o server client
