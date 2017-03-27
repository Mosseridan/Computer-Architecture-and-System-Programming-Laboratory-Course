%macro	syscall3 4
	mov	edx, %4
	mov	ecx, %3
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

section .data
z db 1
x dw 330
y dw 7


global _start
section .text
_start:
		mov edx, 0xf0f0f0f0

		mov ecx, 32
		L: rcl edx, 1
				mov bl, 1
			 sbb bl ,0
			 or dl, bl
			 loop L, ecx

		mov eax, 1
		int 0x80
