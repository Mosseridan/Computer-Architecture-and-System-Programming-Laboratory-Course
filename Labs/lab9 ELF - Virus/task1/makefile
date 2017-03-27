progName1 = task1
srcName1 = task1

all: $(progName1)

$(progName1): $(srcName1).s
	nasm -f elf32 $(srcName1).s -o $(srcName1).o
	ld -melf_i386 $(srcName1).o -o $(progName1)

.PHONY: clean
clean:
	rm -f *.o $(progName)
