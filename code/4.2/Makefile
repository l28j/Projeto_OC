CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L2Cache.c -o $(TARGET)

test:
	./$(TARGET) > 4.2_results.txt
	diff 4.2_results.txt results_L2_1W.txt
clean:
	rm $(TARGET)
	rm 4.2_results.txt