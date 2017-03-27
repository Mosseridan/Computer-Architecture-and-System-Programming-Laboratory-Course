all: ass3

ass3: ass3.o coroutines.o scheduler.o printer.o
	gcc -m32 -g -Wall -o ass3 ass3.o coroutines.o scheduler.o printer.o

ass3.o: ass3.c
	gcc -g -Wall -m32 -ansi -c -o ass3.o ass3.c

coroutines.o: coroutines.s
	nasm -g -f elf -w+all -o coroutines.o coroutines.s -l listing.text

scheduler.o: scheduler.s
	nasm -g -f elf -w+all -o scheduler.o scheduler.s

printer.o: printer.s
	nasm -g -f elf -w+all -o printer.o printer.s

.PHONY: clean
clean:
	rm -f *.o ass3
