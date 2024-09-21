CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L2Cache.c -o $(TARGET)

test:
	./$(TARGET) > 4.2_results.txt

clean:
	rm $(TARGET)