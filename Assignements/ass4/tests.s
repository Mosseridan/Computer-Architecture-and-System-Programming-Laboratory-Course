section .data
Foos:
	mov ebx, [Fob]
	mov [Foa], bx
	add eax, eax

Foa:
	dec eax
	call Foos

Fob:
	ret
	mov eax, 0x4040ea50

global _start
section .text
_start:

	call Foos

	mov eax, 1
	int 0x80
