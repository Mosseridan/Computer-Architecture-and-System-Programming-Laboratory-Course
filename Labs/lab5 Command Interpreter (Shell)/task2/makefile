progName = myshell
srcName1 = task2
srcName2 = LineParser
gccFlags = -g -Wall -m32 -ansi -o
all: $(progName)

$(progName): $(srcName1).o $(srcName2).o
	gcc $(gccFlags) $(progName) $(srcName1).o $(srcName2).o

$(srcName1).o: $(srcName1).c
	gcc -c $(gccFlags) $(srcName1).o $(srcName1).c

$(srcName2).o: $(srcName2).c
	gcc -c $(gccFlags) $(srcName2).o $(srcName2).c


.PHONY: clean
clean:
	rm -f *.o $(progName)
