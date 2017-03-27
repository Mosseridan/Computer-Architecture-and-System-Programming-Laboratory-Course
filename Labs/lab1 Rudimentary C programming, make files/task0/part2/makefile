all: my_echo

my_echo: my_echo.o
	gcc -g -Wall -m32 -o my_echo my_echo.o

my_echo.o: my_echo.c
	gcc -g -Wall -m32 -ansi -c -o my_echo.o my_echo.c

.PHONY: clean
clean:
	rm -f *.o my_echo
