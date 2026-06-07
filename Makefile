CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

.PHONY: all test clean

all: tap_test

tap_test: main.c tap.h
	$(CC) $(CFLAGS) -o $@ main.c -lm

test: tap_test
	./tap_test

clean:
	rm -f tap_test
