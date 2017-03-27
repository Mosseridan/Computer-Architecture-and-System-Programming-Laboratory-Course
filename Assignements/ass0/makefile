all: myexe.out

myexe.out: main.c myelf.o
	gcc -m32 main.c myelf.o -o myexe.out

myelf.o: myasm.s
	nasm -f elf myasm.s -o myelf.o

.PHONY: clean

clean:
	rm -f myelf.o myexe.out