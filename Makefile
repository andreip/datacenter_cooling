CC=gcc
CFLAGS=-Wall -O3

all:
	$(CC) $(CFLAGS) datacenter_cooling.c

clean:
	rm a.out
