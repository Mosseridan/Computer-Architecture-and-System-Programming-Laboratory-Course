section	.rodata
LC0:
	DB	"%s", 10, 0	; Format string

section .bss
LC1:
	RESB	32

section .text
	align 16
	global my_func
	extern printf

my_func:
	push	ebp
	mov	ebp, esp	; Entry code - set up ebp and esp
	mov ecx, dword [ebp+8]	; Get argument (pointer to string)

	pushad			; Save registers

	.remove_leading_zeros:	;remove all leading zeros
	cmp byte [ecx], '0'			; compare next char to '0'
	jnz .start_convertion		; if not '0' coontinue with convertion
	inc ecx									; else move to next char
	jmp .remove_leading_zeros	; and check if it zero

	.start_convertion:
	mov edx, LC1						; make edx point to start of output string
 	push ecx 								;	backup pointer to start of input string
	mov eax, 0 							; initialize counter to 0

	.count_input_length:			; counts the length of the input string
		inc eax 	 							; increment counter
		inc ecx									; increment to point at the next char in the input string
		cmp byte [ecx], 10			; check for NULL terminator character
		jnz .count_input_length  ; if end of string hasent been reached, continue counting
		pop ecx									; restore ecx to point at the begining of the input string
		test al,1								; check the evanity of the input strings length
    jz .is_even
			mov al, [ecx]						; if odd, copy the first char as is
			mov [edx], al						; copy the first char to the output string
			inc ecx									; increment ecx to point at the next char in the input string
			inc edx									; increment edx to point at the next char in the output string

	.is_even:
		mov eax, 0							; reset eax with 0
														; convert the next two chars (in base 4) from input string to a hex number
	.next_digit:							; and append to the output string (at this point it is known that the input strings length is even)
		mov al, [ecx]						; get next char from input string
		sub al, '0'							; convert from ascii representation to number value
		shl al, 2								; multiply by 4
		inc ecx									; point to next char in input
		add al, [ecx]						; add the next char in input to the previus one
		sub al, '0'							; convert from ascii representation to number value
		cmp al, 9								; check whether the nuber (in hex) is between 0-9 or A-F
		JBE .number
			add al, 7							; if between A-F correct nuber so after adding '0' wiil become upper case letter
	.number:
		add al, '0'							; convert back to ascii representation
		.there:
		mov [edx], al						; append to the output string
		inc edx									; point to next char in output strings
		inc ecx									; point to next char in input string
		cmp byte [ecx], 10				; check next char for NULL terminator char
		jnz .next_digit					; if next char is not NULL continue reading next two chars

	push	LC1		; Call printf with 2 arguments: pointer to str
	push	LC0		; and pointer to format string.
	call	printf
	add 	esp, 8		; Clean up stack after call

	popad			; Restore registers
	mov	esp, ebp	; Function exit code
	pop	ebp
	ret
