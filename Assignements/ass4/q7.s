global _start

section .rodata
BitTbl: dw 1, 2, 4, 8
			dw 10h, 20h, 40h, 80h
			dw 100h, 200h, 400h, 800h
			dw 1000h, 2000h, 4000h, 8000h

section .text
_start:
	mov ax, 1010101010101010b
	mov bx, 1
	b1:
	call Member
	b2:
	mov bx, 2
	call Member
	b3:

	mov eax, 1
	int 0x80

Delete:
	and ebx, 0xFFFF ; clear upper 16 bits of ebx
	mov cx, [BitTbl+2*ebx]
	and cx, ax ; make sure the given number is in the set
	xor ax, cx ; delete requested bit (number)
	ret

Odd:
	test ax, 0101010101010101b ; will set the zero flag to 1 if there are no even numbers in the set
	ret

Member:
	and ebx, 0xFFFF ; clear upper 16 bits of ebx
	test ax, [BitTbl+2*ebx] ; if the set dos not contain the given number zero flag will be set
	ret

UnionSets:
	or ax, bx
	ret

Intersection:
	and ax, bx
	ret

Complement:
	not ax
	ret
