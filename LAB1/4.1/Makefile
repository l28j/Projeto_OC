CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)

test:
	./$(TARGET) > 4.1_results.txt
	diff 4.1_results.txt results_L1.txt

clean:
	rm $(TARGET)
	rm 4.1_results.txt