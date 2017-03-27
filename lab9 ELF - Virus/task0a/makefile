progName1 = task0a
progName2 = myELF
srcName1 = task0a
srcName2 = myELF
gccFlags = -g -Wall -m32 -ansi -o

all: $(progName1) $(progName2)

$(progName1): $(srcName1).c
	gcc $(gccFlags) $(progName1) $(srcName1).c

$(progName2): $(srcName2).c
	gcc $(gccFlags) $(progName2) $(srcName2).c

.PHONY: clean
clean:
	rm -f *.o $(progName)
