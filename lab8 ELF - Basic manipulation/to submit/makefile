progName1 = myELF1
progName2 = myELF2
progName3 = myELF3
srcName1 = task1
srcName2 = task2
srcName3 = task3
gccFlags = -g -Wall -m32 -ansi -o

all: $(progName1) $(progName2) $(progName3)

$(progName1): $(srcName1).c
	gcc $(gccFlags) $(progName1) $(srcName1).c

$(progName2): $(srcName2).c
	gcc $(gccFlags) $(progName2) $(srcName2).c

$(progName3): $(srcName3).c
	gcc $(gccFlags) $(progName3) $(srcName3).c

.PHONY: clean
clean:
	rm -f *.o $(progName1) $(progName2) $(progName3)
