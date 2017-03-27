#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: run

# Tool invocations
run: tests.o
	gcc -m32 -g -Wall -o tests tests.o

tests.o: tests.c
	gcc -g -Wall -m32 -ansi -c -o tests.o tests.c 

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o tests
