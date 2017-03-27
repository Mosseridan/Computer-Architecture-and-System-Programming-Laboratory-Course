progName1 = task3
srcName1 = task3
progName2 = virusDetector
srcName2 = task4

all: $(progName1) $(progName2)

$(progName1): $(srcName1).s
	nasm -f elf32 $(srcName1).s -o $(srcName1).o
	ld -melf_i386 $(srcName1).o -o $(progName1)

$(progName2): $(srcName2)
	gcc -g -Wall -m32 -ansi -o $(progName2) $(srcName2).c


.PHONY: clean
clean:
	rm -f *.o $(progName1) $(progName2)
