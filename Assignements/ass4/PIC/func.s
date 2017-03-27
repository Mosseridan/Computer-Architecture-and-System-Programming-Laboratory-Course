section	.rodata
	extern x



section .text

	align 16
	global my_func
	global my_pic_func
	global my_strict_pic_func
	global Start_pic
	extern printf
	extern fgets

Start_pic:
str2:   db "my PIC function", 10, 0
str1:   db "anchor label value =%lx, ip=%lx", 10, 0
Functions:
to_printf: dd printf
to_gets:   dd fgets

get_my_loc:
	call	anchor
anchor: pop	edx
	ret

my_strict_pic_func:
	pusha
	call	get_my_loc	; call to get IP in edx
	call	my_pic
	popa
	ret

my_pic:
	add edx, (str2-anchor)
	mov eax, 4	; write
	mov ebx, 1	; stdout
	mov ecx, edx    ; address of data
	mov edx, 16
	int 0x80
	ret

my_pic_func:
	pusha
	call	get_my_loc
	push	edx
	push	anchor
	push	str1
	mov     edx, printf
	b7:
	call	edx
	add	esp, 12
	popa
	ret

my_func:
	pusha
	call	get_my_loc
	push	edx
	push	anchor
	push	str1
	b8:
	call	printf
	add	esp, 12
	popa
	ret
