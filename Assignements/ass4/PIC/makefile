try:    main.c func.o
	cc -g -m32 main.c func.o -o try


func.o:  func.s
	nasm -f elf func.s -l pic_listing.txt
