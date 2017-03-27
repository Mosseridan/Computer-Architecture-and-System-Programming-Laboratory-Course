global _start

X equ 0x70
Y equ 0x71
Z equ 0x72
ANS equ 0x73

section .text
_start:

	mov al, [X]
	mov bl, [Y]

		test al, 1
		jz x_even
			mov al, 0
	x_even:
		test bl, 1
		jz y_even
			mov bl, 0
	y_even:
		test cl, 1
		jz z_even
			mov cl, 0
	z_even:
		cmp al, bl
		jae compare_z
			mov al, bl
	compare_z:
		cmp al, cl
		jae save_ans
			mov al, cl
	save_ans:
		mov byte [ANS], al

	mov al, 1
	int 0x80
