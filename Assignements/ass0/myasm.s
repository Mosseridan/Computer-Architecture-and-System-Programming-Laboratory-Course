section .data                    	; data section, read-write
        an:    DD 0              	; this is a temporary var

section .text                    	; our code is always in the .text section
        global add_Str_N          	; makes the function appear in global scope
        extern printf            	; tell linker that printf is defined elsewhere 				; (not used in the program)

add_Str_N:                        	; functions are defined as labels
        push    ebp              	; save Base Pointer (bp) original value
        mov     ebp, esp         	; use base pointer to access stack contents
        pushad                   	; push all variables onto stack
        mov ecx, dword [ebp+8]	; get function argument

;;;;;;;;;;;;;;;; FUNCTION EFFECTIVE CODE STARTS HERE ;;;;;;;;;;;;;;;; 
		
		mov		dword [an], 0		; initialize answer
		change_letter:
				add byte [ecx], 8

				cmp byte [ecx], 123
				jae high_symbols
				
				cmp byte [ecx], 97
				jae get_next

				cmp byte [ecx], 91
				jae low_symbols
				jb get_next

		high_symbols:
			cmp byte [ecx], 131
			jb found
			jae get_next
		low_symbols:
			cmp byte [ecx], 99
			jb found
			jae get_next

		found:
			inc dword [an]	

		get_next:
				inc ecx				; increment pointer
				cmp byte [ecx], 0	; check ifbyte pointed to is zero
				jnz change_letter	; keep looping until it is null terminated

;;;;;;;;;;;;;;;; FUNCTION EFFECTIVE CODE ENDS HERE ;;;;;;;;;;;;;;;; 
         popad                    ; restore all previously used registers
         mov     eax,[an]         ; return an (returned values are in eax)
         mov     esp, ebp
         pop     ebp
         ret 
