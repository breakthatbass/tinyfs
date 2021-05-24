CC=gcc
CFLAGS=-fsanitize=address -Wall -g

all: send recv

send: send.c
	$(CC) $(CFLAGS) send.c -o send

recv: recv.c
	$(CC) $(CFLAGS) recv.c -o recv

install: send recv
	cp send /usr/local/bin/send
	cp recv /usr/local/bin/recv

clean:
	rm send recv
	rm -rf *.dSYM