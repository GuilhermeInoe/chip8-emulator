CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic

all: chip8

chip8: c8vm.o main.o
	$(CC) $(CFLAGS) -o chip8 c8vm.o main.o

c8vm.o: c8vm.c c8vm.h
	$(CC) $(CFLAGS) -c c8vm.c

main.o: main.c c8vm.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o chip8
