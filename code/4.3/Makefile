CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L2CacheW.c -o $(TARGET)

test:
	./$(TARGET) > 4.3_results.txt
	diff 4.3_results.txt results_L2_2W.txt

clean:
	rm $(TARGET)
	rm 4.3_results.txt