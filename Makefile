CC=c++

CFLAGS=-std=c++14

all:
	$(CC) $(CFLAGS) main.cpp -o server
