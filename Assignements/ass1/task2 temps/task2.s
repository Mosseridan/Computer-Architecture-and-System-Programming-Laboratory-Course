section .rodata
LC0:
  DB "%llX", 10, 0 ; format unsigned long long hex

section .bss
LC1:
  RESB 8

section .text
  align 16
  global calc_func
  extern printf

calc_func:
  push ebp
  mov ebp, esp  ; Entry code - set up ebp and esp
  mov ecx, dword [ebp+8] ; Get argument (unsigned long long)

pushad       ; Save registers
stop:
  ;MY code here !!!!!!!!!!

  mov edx, LC1
  mov eax, [ecx]
  mov [edx], eax
  add edx, 4
  add ecx, 4
  mov eax, [ecx]
  mov [edx], eax
  inc byte [LC1]
  add byte [LC1+1], 1
  push dword [LC1+2]
  push dword [LC1]		; Call printf with 2 arguments: pointer to unsigned long long hex
  push	LC0		          ; and pointer to format unsigned long long hex.
end:
  call	printf
  add 	esp, 8		; Clean up stack after call

  popad			; Restore registers
  mov	esp, ebp	; Function exit code
  pop	ebp
  ret
