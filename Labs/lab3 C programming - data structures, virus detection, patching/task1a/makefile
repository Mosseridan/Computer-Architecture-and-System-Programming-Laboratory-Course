progName = task1a
srcName = task1a.c

all: $(progName)

$(progName): $(srcName)
	gcc -g -Wall -m32 -ansi -o $(progName) $(srcName)

.PHONY: clean
clean:
	rm -f $(progName)
