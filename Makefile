CC=gcc
CFLAGS=-Wall -Wextra -std=c99

TARGETS = main

build: $(TARGETS)

main: main.c
	$(CC) $(CFLAGS) main.c -o main

clean:
	rm -f $(TARGETS)