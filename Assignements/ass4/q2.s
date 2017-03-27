global _start
section .text
get_my_loc:
	pop ecx
	jmp anchor
_start:
		xor eax, eax
		xor ebx, ebx
		xor edx, edx
		call get_my_loc
	anchor:
		add ecx, msg1-anchor
		mov dl, 6
		mov bl, 1
		mov al, 4
		int 0x80
	next:
		mov al, 1
		int 0x80
msg1: db "thank you!"
