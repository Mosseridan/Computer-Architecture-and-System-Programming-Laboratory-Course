;;;;;;;;;;; Macros ;;;;;;;;;;;
%macro init_func 0
  push ebp           ; save previous value of ebp
  mov ebp, esp       ; set ebp to point to myFunc frame
%endmacro

%macro return_from_func 0
  mov esp, ebp      ; "delete" local variables of myFunc
  pop ebp           ; restore previous value of ebp
  RET               ; return to the caller
%endmacro

%macro print 2       ; call printf
  pusha
  push dword %1
  push dword %2
  call printf
  add esp, 8
  popa
%endmacro

%macro err_exit 1
  print %1, FORMAT_STRING
  mov eax, 1
  xor ebx, ebx
  int 80h
%endmacro

;read 1 byte from file
%macro read_byte 0
    pusha
    mov eax, 3
    mov ebx, dword [FD]
    mov ecx, BUFF
    mov edx, 1
    int 0x80
    cmp eax, 0
    jne %%.read_ok
    err_exit ERR_EOF
    %%.read_ok:
    popa
%endmacro

%macro call_malloc 2
  pusha
  push %1
  call malloc
  mov %2, eax
  add esp, 4
  popa
%endmacro

%macro call_free 1
  pusha
  push %1
  call free
  add esp, 4
  popa
%endmacro

%macro allocate_co 4
  pusha
  call_malloc CO_SIZE, %1
  mov eax, %1
  mov dword [eax+CODEP], %2
  mov dword [eax+FLAGSP], 0
  mov dword [eax+ARG1], %3
  mov dword [eax+ARG2], %4
  mov ecx, eax
  add ecx, CO_SIZE
  mov dword [eax+SPP], ecx
  popa
%endmacro


STKSZ equ 16*1024        ; co-routine stack size
CODEP equ 0              ; offset of pointer to co-routine function in co-routine structure
FLAGSP equ 4             ; offset of pointer to flags co-routine structure
ARG1  equ 8
ARG2 equ 12
SPP equ 16               ; offset of pointer to co-routine stack in co-routine structure
CO_SIZE equ STKSZ+20
BUFF_SIZE equ 2

;;;;;;;;;;; Read only data ;;;;;;;;;;;
section .rodata
  align 16
  global FORMAT_INT
  FORMAT_INT: db "%d ",0      ; int formating string
  FORMAT_STRING:  db "%s",10, 0      ; string formating string
  FORMAT_CHAR:  db "%c", 10,0      ; char formating string
  NL: db " ",0
  ERR_ARGS: db "Error incorrect args, expected: [optional -d] <filename> <length> <width> <t> <k>",0
  ERR_OPEN: db "Error opening file. exiting...", 0
  ERR_EOF: db "Error reached end of file before expected. exiting...",0
  TEN: dd 10
  CORS: dd 0

;;;;;;;;;;; Initialized data ;;;;;;;;;;;
section .data
  align 16
  FD: dd -1
  global DEBUG
  DEBUG: dd 0
  global WorldWidth
  WorldWidth: dd 0
  global WorldLength
  WorldLength: dd 0
  global NumGen
  NumGen: dd 0
  global NumCo
  NumCo: dd 0
  global NumCells
  NumCells: dd 0
  global PrintRate
  PrintRate: dd 0
  global state
  state: dd 0
  global cors
  cors: dd 0
  global cells
  cells: dd 0
  global scheduler
  scheduler: dd 0
  global printer
  printer: dd 0

;;;;;;;;;;; Uninitialized data ;;;;;;;;;;;
section .bss
  align 16
  CURR: resd 1
  SPT: resd 1        ; temporary stack pointer variable
  SPMAIN: resd 1     ; stack pointer of main
  STK: resb STKSZ
  STATE: resd 2
  STATE_CONT: resd 2
  BUFF: resb BUFF_SIZE

;;;;;;;;;;; CODE ;;;;;;;;;;;
section .text
  align 16
  extern printf
  extern fgets
  extern malloc
  extern free
  extern secheduler_func
  extern printer_func
  global build_structures
  global init_co_from_c
  global start_co_from_c
  global resume
  global end_co
  global clear_data

build_structures:
  mov eax, [ebp+8]  ;get argc
  mov ebx, [ebp+12] ; get argv
  init_func
    ;check debug flag
    cmp eax, 7        ; if argc = 7 and argv[1] = -d then debug flag is on
    jne .no_debug
      mov ecx, dword [ebx + 4]      ; get argv[1]
      cmp byte [ecx], '-'
      jne .no_debug
      cmp byte [ecx+1], 'd'
      jne .no_debug
      mov dword [DEBUG], 1
      add ebx, 4
      dec eax

    .no_debug:
    ; check correct number of atgs
    cmp eax, 6        ; there needs to be 6 args
    je .open_file
      ; print an error and exit
      err_exit ERR_ARGS

    .open_file:
      ;open the file for reading
      pusha
      mov eax, 5
      mov ebx, dword [ebx + 4]  ;get argv[2] (file name)
      mov ecx, 0          ;for read only access
      mov edx, 0777       ;read, write and execute by all
      int  0x80
      cmp eax, 0    ; if file did not open correctly print error and exit
      jge .open_ok
      ; print an error and exit
      err_exit ERR_OPEN
      .open_ok:
      mov  [FD], eax
      popa

    ; set WorldLength
    push dword [ebx + 8]
    call atoi
    mov dword [WorldLength], eax

    ; set WorldWidth
    push dword [ebx + 12]
    call atoi
    mov dword [WorldWidth], eax

    ; set NumGen
    push dword [ebx + 16]
    call atoi
    mov dword [NumGen], eax

    ; set PrintRate
    push dword [ebx + 20]
    call atoi
    mov dword [PrintRate], eax

    ; set NumCells
    mov eax, dword [WorldLength]
    mul dword [WorldWidth]
    mov dword [NumCells], eax

    ; allocate state
    call_malloc eax, dword [state]; eax still holds NumCells

    mov ecx, 0
    mov edx, dword [state]

    ; read file into state
    .read_line:
      mov ebx, 0
      inc ecx
      .read_cell:
      test ecx, 1
      jz .even_line
        ; odd line
        read_byte       ; read state of the next cell as char
        push dword BUFF
        call atoi       ; convert to int
        mov byte [edx+ebx], al
        read_byte ; read through the deviding space char (" ")
        jmp .cell_done
      .even_line:
        read_byte       ; read through the deviding space char (" ")
        read_byte ; read state of the next cell as char
        push dword BUFF
        call atoi       ; convert to int
        mov byte [edx+ebx], al
      .cell_done:
        inc ebx
        cmp ebx, dword [WorldWidth]
        jnz .read_cell
      add edx, dword [WorldWidth]
      cmp ecx, dword [WorldLength]
      jz .read_end
      .seek_to_end_of_line:
        read_byte ; read through the deviding new line char

        cmp byte [BUFF], 10
        jnz .seek_to_end_of_line
      jmp .read_line
      .read_end:

    ; allocate cors and set NumCo
    mov eax, dword [NumCells]
    add eax, 2
    mov dword [NumCo], eax
    shl eax, 2
    call_malloc eax, dword [cors]

    ; set cells
    mov ebx, dword[cors]
    add ebx, 8
    mov dword [cells], ebx

    ; allocate scheduler co-routine and set a pointer to it in cors
    mov ebx, dword[cors]  ; set ebx to point to start of cors
    mov ecx, dword[NumGen]
    mov edx, dword[PrintRate]
    allocate_co dword [ebx], secheduler_func, ecx, edx
    mov ecx, dword [ebx]
    mov dword [scheduler], ecx

    ; allocate printer co-routine and set a pointer to it in cors
    add ebx, 4
    mov ecx, dword[WorldLength]
    mov edx, dword[WorldWidth]
    allocate_co dword [ebx], printer_func, ecx, edx
    mov ecx, dword [ebx]
    mov dword[printer], ecx

    ; allocate space for all cell co-routines and set pointers to them in cors
    add ebx, 4
    mov edx, 0
    .next_row:
      mov ecx, 0
      .next_cell:
        ;print edx, FORMAT_INT
        allocate_co dword [ebx], cell_func, ecx, edx
        add ebx, 4
        add ecx, 1
        cmp ecx, dword [WorldWidth]
        jnz .next_cell
      inc edx
      cmp edx, dword [WorldLength]
      jnz .next_row
  return_from_func



atoi:
  init_func
    push ecx
    push edx
    push ebx
    mov ecx, dword [ebp+8]  ; Get argument (pointer to string)
    xor eax,eax
    xor ebx,ebx
  .atoi_loop:
    xor edx,edx
    cmp byte[ecx],0
    jz  .atoi_end
    imul dword [TEN]
    mov bl,byte[ecx]
    sub bl,'0'
    add eax,ebx
    inc ecx
    jmp .atoi_loop
  .atoi_end:
    pop ebx                 ; Restore registers
    pop edx
    pop ecx
  return_from_func


init_co_from_c:
  init_func
    mov ebx, dword [ebp+8] ; ebx contains the index of the co-routine to be initialized
    mov eax, dword [cors]
    mov ebx, dword [eax + 4*ebx]
    call co_init
  return_from_func


co_init:
    pusha
    bts dword [ebx+FLAGSP],0 ; test if already initialized
    jc .init_done
    mov eax, dword [ebx+CODEP] ; get initial PC
    mov [SPT], esp ; save original SP
    mov esp, dword [ebx+SPP] ; get initial SP
    push dword[ebx+ARG2]  ; push second argument
    push dword[ebx+ARG1]  ; push first argument
    push 0
    push 0
    mov ebp, esp ; also use as EBP
    push eax ; push initial "return" address (initial PC)
    pushfd ; push flags
    pushad ; push all other registers
    mov [ebx+SPP],esp ; save new SP in structure
    mov esp, [SPT] ; restore original SP
  .init_done:
    popa
    ret

start_co_from_c:
  init_func
  pusha
  mov dword[SPMAIN], esp
  mov ebx, dword[scheduler] ; get a pointer to the scheduler structure
  jmp do_resume

end_co:
  mov esp, dword [SPMAIN]
  popa
  return_from_func


resume:       ;save caller state
  pushf
  pusha
  mov edx, dword [CURR]
  mov dword [edx+SPP], esp ; save current SP

do_resume:  ; load SP in order to resume co-routine
  mov esp, dword [ebx+SPP]
  mov [CURR], ebx
  popa   ; restore resumed co-routine state
  popf
  ret     ; "return" to resumed co-routine

clear_data:
  init_func
  ; free state
  call_free dword [state]
  ; free cors
  mov ebx, dword [cors]
  mov ecx, 0
  .free_co:
    call_free dword[ebx+4*ecx]
    inc ecx
    cmp ecx, dword[NumCo]
    jnz .free_co
  call_free ebx
  return_from_func


cell_func:
  mov ebx, dword [state]      ; get pointer to satrt of sate
  mov eax, 0
  mov ecx, 0

  print [ebp+12], FORMAT_INT
  print [ebp+8], FORMAT_INT




  mov al, byte[ebp+12]     ; get y
  test al, 1
  jz .even_line
  ; odd line
    ; check top left neigbor odd line
    cmp al, 0
    jne .top_left_odd     ; if y=0 top neigbor is in to bottom row
      mov al, byte [WorldLength]
    .top_left_odd:
      dec al                ; get above row index
      mul byte[WorldWidth]  ; get offset to start of above row in state
      add al, byte[ebp+8]   ; get offset to the x'th item in the above row (top left neigbor)
      add cl, byte[ebx+eax] ; if top left neigbor is alive, increment neigbor count
      print 111, FORMAT_INT
      print ecx, FORMAT_INT
      print eax,FORMAT_INT

    ; check top right neigbor odd line
    mov dl, byte [WorldWidth]
    dec dl
    cmp byte[ebp+8], dl
    jne .top_right_odd
      sub al, byte[WorldWidth]
    .top_right_odd:
      inc al
      add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count
      print 222, FORMAT_INT
      print ecx, FORMAT_INT
      print eax,FORMAT_INT

    ; check bottom left neigbor odd line
    mov al, byte [ebp+12] ; get y
    mov dl, byte [WorldLength]
    dec dl
    cmp byte [ebp+12], dl
    jne .bottom_left_odd
      mov al, -1
    .bottom_left_odd:
      inc al
      mul byte [WorldWidth]
      add al, byte [ebp+8]
      add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

    ;check bottom right neigbor odd line
    mov dl, byte [WorldWidth]
    dec dl
    cmp byte[ebp+8], dl
    jne .bottom_right_odd
      sub al, byte[WorldWidth]
    .bottom_right_odd:
      inc al
      add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

jmp .left_right

.even_line:
  ; check top right neigbor even line
  cmp al, 0
  jne .top_right_even     ; if y=0 top neigbor is in to bottom row
    mov al, byte [WorldLength]
  .top_right_even:
    dec al                ; get above row index
    mul byte[WorldWidth]  ; get offset to start of above row in state
    add al, byte[ebp+8]   ; get offset to the x'th item in the above row (top left neigbor)
    add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

  ; check top left neigbor even line
  cmp byte[ebp+8], 0
  jne .top_left_even
    add al, byte[WorldWidth]
  .top_left_even:
    dec al
    add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

  ; check bottom right neigbor even line
  mov al, byte [ebp+12] ; get y
  mov dl, byte [WorldLength]
  dec dl
  cmp byte [ebp+12], dl
  jne .bottom_right_even
    mov al, -1
  .bottom_right_even:
    inc al
    mul byte [WorldWidth]
    add al, byte [ebp+8]
    add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

  ;check bottom left neigbor even line
  cmp byte[ebp+8], 0
  jne .bottom_left_even
    add al, byte [WorldWidth]
  .bottom_left_even:
    dec al
    add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

.left_right:
; check left neigbor
mov al, byte[ebp+12]        ; get y
mul byte [WorldWidth]       ; get offset to start of the y'th row in state
add al, byte [ebp+8]
cmp byte [ebp+8], 0
jne .left
  add al, byte [WorldWidth]
.left:
  dec al
  add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

; check right neigbor
mov al, byte[ebp+12]      ; get y
mul byte [WorldWidth]     ; get offset to start of the y'th row in state
add al, byte [ebp+8]
mov dl, byte [WorldWidth]
dec dl
cmp byte [ebp+8], dl
jne .right
  sub al, byte [WorldWidth]
.right:
  inc al
  add cl, byte[ebx+eax] ; if top right neigbor is alive, increment neigbor count

; check current state
mov al, byte [ebp+12]
mul byte [WorldWidth]
add al, byte [ebp+8]
mov edx, 0
cmp byte [ebx+eax], 1
je .alive
  ; current cell is currently dead
  cmp ecx, 2
  jne .update_cell
    mov dl, 1
    jmp .update_cell
.alive:
  ; current cell is currently alive
  cmp ecx, 3
  jl .update_cell
  cmp  ecx, 4
  jg .update_cell
  mov dl, 1
.update_cell:

  mov ebx, dword[scheduler]   ; resume scheduler
  call resume

  mov ebx, dword[state]
  mov byte [ebx+eax], dl      ; update cell state in state

  mov ebx, dword[scheduler]   ; resume scheduler
  call resume

jmp cell_func                 ; start func from the bgginig
