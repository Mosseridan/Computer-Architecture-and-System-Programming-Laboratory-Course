progName = hexeditplus
srcName1 = task1a
gccFlags = -g -Wall -m32 -ansi -o

all: $(progName)

$(progName): $(srcName1).c
	gcc $(gccFlags) $(progName) $(srcName1).c

.PHONY: clean
clean:
	rm -f *.o $(progName)
