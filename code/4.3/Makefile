CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L2CacheW.c -o $(TARGET)

test:
	./$(TARGET) > 4.3_results.txt

clean:
	rm $(TARGET)