taskname = task2
srcName = oop.c

all: $(taskname)

$(taskname): $(srcName)
	gcc -g -Wall -m32 -ansi -o $(taskname) $(srcName)

.PHONY: clean
clean:
	rm -f $(taskname)
