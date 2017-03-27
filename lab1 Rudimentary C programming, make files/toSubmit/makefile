taskname = task2

all: filter

filter: $(taskname).o
	gcc -g -Wall -m32 -o filter $(taskname).o

$(taskname).o: $(taskname).c
	gcc -g -Wall -m32 -ansi -c  -o $(taskname).o $(taskname).c


task1C: task1C.o
	gcc -g -Wall -m32 -o filter task1C.o

task1C.o: task1C.c
	gcc -g -Wall -m32 -ansi -c  -o task1C.o task1C.c


.PHONY: clean
clean:
	rm -f *.o filter


