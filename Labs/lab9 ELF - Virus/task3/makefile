progName1 = task3
srcName1 = task3

all: $(progName1)

$(progName1): $(srcName1).s
	nasm -f elf32 $(srcName1).s -o $(srcName1).o -l listing.txt
	ld -melf_i386 $(srcName1).o -o $(progName1)

.PHONY: clean
clean:
	rm -f *.o $(progName)
