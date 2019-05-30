CC=c++

CFLAGS=-std=c++14

all:
	$(CC) $(CFLAGS) main.cpp server.cpp -o server
