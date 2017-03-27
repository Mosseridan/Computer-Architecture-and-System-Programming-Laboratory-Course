
extern cells
extern printer
extern NumCells
extern resume
extern end_co
global secheduler_func

secheduler_func:
  mov ecx, dword [ebp+8] ; get number of generations
  shl ecx, 2 ; for each generation we resume every cell twice
  mov edx, dword [ebp+12] ; get print reate
  mov eax, 0

  .next_gen:
    .next_resume:
      cmp edx, 0
      jnz .resume_cell
        mov ebx, dword[printer]
        call resume
        mov edx, dword [ebp+12] ; reset print reate counter
      .resume_cell:
        mov ebx, dword [cells]  ; get pointer to cells
        mov ebx, dword [ebx+4*eax] ; get pointer to the next cells structure from cells
        call resume   ; resume next cell
        inc eax ; inc cell iterator
        dec edx     ; decrese
        cmp eax, dword [NumCells] ; if iterated through all cells
        jnz .next_resume
      mov eax, 0  ; reset cell iterator
    loop .next_gen, ecx

  jmp end_co
