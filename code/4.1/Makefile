CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)

test:
	./$(TARGET) > 4.1_results.txt

clean:
	rm $(TARGET)