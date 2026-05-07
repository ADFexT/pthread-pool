CC = gcc

CFLAGS = -Wall -pthread

TARGET = Threadpool

SRC = main.c Threadpool.c

.PHONY: clean all run

all:$(TARGET)

$(TARGET):$(SRC)
	$(CC) $(CFLAGS) $^ -o $@

run:$(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

