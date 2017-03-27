%macro print 2       ; call printf
  pusha
  push dword %1
  push dword %2
  call printf
  add esp, 8
  popa
%endmacro

section .rodata
FORMAT_INT: db "%d",0      ; int formating string
FORMAT_STRING: db "%s",0
NL: db "",10,0
SPACE: db " ",0

section .text
extern printf
extern state
extern scheduler
extern resume
global printer_func
global print_state

printer_func:
  push dword[ebp+12]
  push dword[ebp+8]
  call print_state
  add esp,8
  mov ebx, dword [scheduler]
  call resume
 jmp printer_func

print_state:
push ebp           ; save previous value of ebp
mov ebp, esp       ; set ebp to point to myFunc frame

mov ebx, dword [state] ; get pointer to state
mov edx, 0
mov eax, 0
.next_row:
  mov ecx, 0
  test edx, 1
  jz .even_row
  ; print odd line (counting from 0)
  .odd_row:
    print SPACE, FORMAT_STRING
    mov al, byte [ebx+ecx]
    print eax, FORMAT_INT
    inc ecx
    cmp ecx, dword [ebp+12] ; WorldWIdth
    jnz .odd_row
    jmp .row_done
  ; print even line (counting from 0)
  .even_row:
    mov al, byte [ebx+ecx]
    print eax, FORMAT_INT
    print SPACE, FORMAT_STRING
    inc ecx
    cmp ecx, dword [ebp+12] ; WorldWIdth
    jnz .even_row
  ;move to next row if exists such
  .row_done:
  print NL, FORMAT_STRING
  add ebx, ecx
  inc edx
  cmp edx, dword [ebp+8]  ; WorldLength
  jnz .next_row

  mov esp, ebp      ; "delete" local variables of myFunc
  pop ebp           ; restore previous value of ebp
  RET               ; return to the caller
