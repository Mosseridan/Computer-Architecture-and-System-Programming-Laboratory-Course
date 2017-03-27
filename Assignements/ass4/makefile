progName0 = tests
srcName0 = tests
progName1 = q1
srcName1 = q1
progName2 = q2
srcName2 = q2
progName3 = q3
srcName3 = q3
progName7 = q7
srcName7 = q7
progName14 = q14
srcName14 = q14


all: $(progName0) $(progName1) $(progName2) $(progName3) $(progName7) $(progName14)

$(progName0): $(srcName0).s
	nasm -f elf32 $(srcName0).s -o $(srcName0).o -l  $(srcName0)_listing.txt
	ld -melf_i386 $(srcName0).o -o $(progName0)

$(progName1): $(srcName1).s
	nasm -f elf32 $(srcName1).s -o $(srcName1).o -l $(srcName1)_listing.txt
	ld -melf_i386 $(srcName1).o -o $(progName1)

$(progName2): $(srcName2).s
	nasm -f elf32 $(srcName2).s -o $(srcName2).o -l $(srcName2)_listing.txt
	ld -melf_i386 $(srcName2).o -o $(progName2)

$(progName3): $(srcName3).s
	nasm -f elf32 $(srcName3).s -o $(srcName3).o -l $(srcName3)_listing.txt
	ld -melf_i386 $(srcName3).o -o $(progName3)

$(progName7): $(srcName7).s
	nasm -f elf32 $(srcName7).s -o $(srcName7).o -l $(srcName7)_listing.txt
	ld -melf_i386 $(srcName7).o -o $(progName7)

$(progName14): $(srcName14).c
	gcc -g -Wall -m32 -ansi -o $(progName14)  $(srcName14).c

.PHONY: clean
clean:
	rm -f *.o $(progName1) $(progName2) $(progName3) $(progName7)
