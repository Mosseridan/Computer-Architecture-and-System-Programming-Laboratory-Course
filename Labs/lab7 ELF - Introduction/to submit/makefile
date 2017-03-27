progName1 = hexeditplus
progName2 = my_ntsc
srcName1 = task1
srcName2 = task4
gccFlags = -g -Wall -m32 -ansi -o

all: $(progName1) $(progName2)

$(progName1): $(srcName1).c
	gcc $(gccFlags) $(progName1) $(srcName1).c

$(progName2): $(srcName2).c
	gcc $(gccFlags) $(progName2) $(srcName2).c

.PHONY: clean
clean:
	rm -f *.o $(progName)
