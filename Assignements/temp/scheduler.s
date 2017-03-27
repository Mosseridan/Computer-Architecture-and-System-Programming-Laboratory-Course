%macro print 2       ; call printf
  pusha
  push dword %1
  push dword %2
  call printf
  add esp, 8
  popa
%endmacro
extern FORMAT_INT
extern printf

extern cors
extern cells
extern printer
extern NumCells
extern resume
extern end_co
global secheduler_func

secheduler_func:
  mov ebx, dword [cells]
  mov edx, dword [ebp+8] ; get number of generations
  mov ecx, 0

  .next_iteration:
    mov eax, 0
    test ecx, dword [ebp+12]
    jnz .next_cell_first_run
      mov ebx, dword [printer]
      call resume
    .next_cell_first_run:
      mov ebx, dword[cells]
      mov ebx, dword [ebx+4*eax]
      call resume
      inc eax
      cmp eax, dword [NumCells]
      jnz .next_cell_first_run
    ;mov eax, 0
    ;.next_cell_second_run:
    ;  mov ebx, dword [ebx+eax]
    ;  call resume
    ;  inc eax
    ;  cmp eax, dword [NumCells]
    ;  jnz .next_cell_second_run

    inc ecx
    cmp ecx, edx
    jnz .next_iteration

  jmp end_co
