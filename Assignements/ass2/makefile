all: calc.bin

calc.bin: calc.o
	gcc -m32 -Wall -g calc.o -o calc.bin

calc.o: calc.s
	nasm -g -f elf calc.s -o calc.o

.PHONY: clean
clean:
	rm -f *.o calc.bin
