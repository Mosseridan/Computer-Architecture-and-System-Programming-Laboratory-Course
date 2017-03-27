all: adder

adder: main.o numbers.o add.o
	gcc -g -Wall -m32 -o adder main.c numbers.o add.o

main.o: main.c
	gcc -g -Wall -m32 -ansi -c -o main.o main.c

numbers.o: numbers.c
	gcc -g -Wall -m32 -ansi -c -o numbers.o numbers.c

add.o: add.s
	nasm -g -f elf -w+all -o add.o add.s

.PHONY: clean
clean:
	rm -f *.o adder
