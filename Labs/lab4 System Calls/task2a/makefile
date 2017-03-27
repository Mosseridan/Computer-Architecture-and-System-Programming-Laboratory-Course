progName = flame2
nasmName = start
src1Name = util
src2Name = task2a

all: $(progName)

$(progName): $(nasmName).o $(src1Name).o $(src2Name).o
	ld -melf_i386 $(nasmName).o $(src1Name).o $(src2Name).o -o $(progName)

$(src1Name).o: $(src1Name).c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector $(src1Name).c -o $(src1Name).o

$(src2Name).o: $(src2Name).c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector $(src2Name).c -o $(src2Name).o

$(nasmName).o: $(nasmName).s
	nasm -f elf $(nasmName).s -o $(nasmName).o


.PHONY: clean
clean:
	rm -f *.o $(progName)
