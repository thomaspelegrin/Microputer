# specify the compiler
CC=gcc
# specify options for the compiler
CFLAGS=-c -Wall

program: p1.o microputer.o
	$(CC) p1.o microputer.o -o program
p1.o: p1.c microputer.h
	$(CC) $(CFLAGS) p1.c
microputer.o: microputer.c microputer.h
	$(CC) $(CFLAGS) microputer.c
clean:
	rm -rf *o program