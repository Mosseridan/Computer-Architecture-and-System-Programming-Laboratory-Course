progName1 = myshell1
progName2 = mypipeline2
progName3 = mypipeline3
progName4 = myshell4
srcName0 = LineParser
srcName1 = task1
srcName2 = task2
srcName3 = task3
srcName4 = task4
gccFlags = -g -Wall -m32 -ansi -o

all: $(progName1) $(progName2) $(progName3) $(progName4)

#task1
$(progName1): $(srcName1).o $(srcName0).o
	gcc $(gccFlags) $(progName1) $(srcName1).o $(srcName0).o

$(srcName1).o: $(srcName1).c
	gcc -c $(gccFlags) $(srcName1).o $(srcName1).c

$(srcName0).o: $(srcName0).c
	gcc -c $(gccFlags) $(srcName0).o $(srcName0).c

#task2
$(progName2): $(srcName2).c
	gcc $(gccFlags) $(progName2) $(srcName2).c

#task3
$(progName3): $(srcName3).c
	gcc $(gccFlags) $(progName3) $(srcName3).c

#task4
$(progName4): $(srcName4).o $(srcName0).o
	gcc $(gccFlags) $(progName4) $(srcName4).o $(srcName0).o

$(srcName4).o: $(srcName4).c
	gcc -c $(gccFlags) $(srcName4).o $(srcName4).c


.PHONY: clean
clean:
	rm -f *.o $(progName1) $(progName2) $(progName3) $(progName4)
