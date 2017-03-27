all: count-words

count-words: count-words.c
	gcc -g -Wall -Wstrict-prototypes -m32 -o count-words count-words.c

.PHONY: clean

clean:
	rm -f count-words