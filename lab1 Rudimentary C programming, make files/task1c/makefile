taskname = task1c

all: filter

filter: $(taskname).o
	gcc -g -Wall -m32 -o filter $(taskname).o

$(taskname).o: $(taskname).c
	gcc -g -Wall -m32 -ansi -c  -o $(taskname).o $(taskname).c

.PHONY: clean
clean:
	rm -f *.o filter


