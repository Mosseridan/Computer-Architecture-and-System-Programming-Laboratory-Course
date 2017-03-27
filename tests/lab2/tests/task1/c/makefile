taskname = task1C

all: $(taskname)

$(taskname): $(taskname).c
	gcc -g -Wall -m32 -ansi -o $(taskname) $(taskname).c

.PHONY: clean
clean:
	rm -f $(taskname)
