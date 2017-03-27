%macro print 2                                            ; call printf
  push %1
  push %2
  call printf
  add esp, 8
%endmacro

%macro print_err 2                                        ; call fprintf for error
  push %1
  push %2
  push dword [stderr]
  call fprintf
  add esp, 12
%endmacro

%macro init_func 0
  push ebp                    ; save previous value of ebp
  mov ebp, esp                ; set ebp to point to myFunc frame
%endmacro

%macro return_from_func 0
  mov esp, ebp ; "delete" local variables of myFunc
  pop ebp ; restore previous value of ebp
  RET ; return to the caller
%endmacro

%macro get_input 0
  push dword [stdin]
  push BUFFER_SIZE
  push BUFFER
  call fgets
  add esp, 12
%endmacro

%macro is_end? 0
  cmp al, 'q'
  jz end_calc
%endmacro

%macro is_add? 0
  cmp al, '+'
  jz calc_add
%endmacro

%macro is_pop_and_print? 0
  cmp al, 'p'
  jz calc_pop_and_print
%endmacro

%macro is_duplicate? 0
  cmp al, 'd'
  jz calc_duplicate
%endmacro

%macro is_bitwize_and? 0
cmp al, '&'
jz calc_bitwize_and
%endmacro

%macro is_operand? 0
  ;TO COMLETE: CHECK OPERAND CORRECTNESS
  pushad               ; backup registers
  call create_operand  ; create and operand list from input
  push_op eax          ; push created operand to op_stack
  popad                ; restore registers
%endmacro

%macro overflow? 0
  cmp dword [stack_counter], STACK_SIZE
  jnz %%no_overflow
  print_err(ERR_OVERFLOW)
  jmp my_calc_code
  %%no_overflow:
%endmacro

%macro push_op 1
  overflow?
  mov ecx, dword [stack_counter]
  mov [OP_STACK+ecx], %1
  inc dword [stack_counter]
%endmacro

%macro sufficient_args? 1
  cmp dword[stack_counter], %1
  jae %%sufficient_args
  print_err(ERR_NUM_OF_ARGS)
  jmp my_calc_code
  %%sufficient_args:
%endmacro

%macro pop_op 0
  sufficient_args? 1
  mov ecx, dword[stack_counter]
  mov eax, [OP_STACK+ecx]
  dec dword[stack_counter]
%endmacro

%macro count_input_length 0
  mov ecx, BUFFER                     ; store a pointer to the inputs leftmost digit (first char in BUFFER) in ecx
  mov eax, 0                          ; reset eax with 0,the inputs length will be returned in eax
  %%next_digit:			                  ; counts the length of the input string
  cmp byte [ecx], 10			            ; check for NULL terminator character
  jz %%reached_end                     ; if end of string hasent been reached, continue counting
  inc eax 	 							            ; increment counter
  inc ecx								              ; increment to point at the next char in the input string
  jmp %%next_digit
  %%reached_end:
%endmacro

%macro allocate_link 0                                     ; allocating memory with call to malloc
  pushad
  push LINK_SIZE
  call malloc
  mov [return], eax
  add esp, 4
  popad
%endmacro

%macro add_link 2                   ; converts a pait of digits (represented by ascii chars) to a BCD, store them in a link and set it as the head of the list in ebx
  sub %1, '0'                       ; convert left digit from ascii to BCD
  shl %1, 4
  sub %2, '0'                       ; convert right digit from ascii to BCD
  add %1, %2                        ; join the two in to a single number in BCD
  allocate_link                     ; allocate space for a link to store this number
  mov edx, dword[return]            ; move pointer to allocated space from return (variable) to edx
  mov byte [edx], %1                ; store number in link
  mov dword [edx+1], ebx            ; store pointer to the rest of the list in link
  mov ebx, edx                      ; make this link the head of the currently constructed list
%endmacro

%macro reached_end? 1
  cmp %1, 0
  jz .end_operation
%endmacro

%define print_int(i) print i, FORMAT_INT
%define print_char(c) print c, FORMAT_CHAR
%define print_string(s) print s, FORMAT_STRING
%define print_err(err) print_err err, FORMAT_STRING
%define print_two_digits(c) print c, FORMAT_UNSIGNED_CHAR
%define get_data(link) mov al, byte [link]
%define get_next(link) mov link, dword [link+1]

STACK_SIZE equ 5
BUFFER_SIZE equ 80
LINK_SIZE equ 5

section .rodata
FORMAT_INT:
  db "%d", 10,0                                             ; int formating string
FORMAT_CHAR:
  db "%c", 10,0                                             ; char formating string
FORMAT_UNSIGNED_CHAR:
  db "%hhu", 0
FORMAT_STRING:
    db "%s", 0                                              ; string formating string
CALC_PROMPT:
  db "calc: ", 0                                            ; prompt to user
ERR_OVERFLOW:
  db "Error: Operand Stack Overflow", 10, 0                 ; prompt for Operand Stack Overflow
ERR_NUM_OF_ARGS:
  db "Error: Insufficient Number of Arguments on Stack", 10, 0 ; prompt for Insufficient Number of Arguments on Stack

section .data
op_count:
  dd 0
stack_counter:
  dd 0
return:
  dd 0


section .bss
OP_STACK:
  resd STACK_SIZE
BUFFER:
  resb BUFFER_SIZE

section .text
     align 16
     global main
     extern printf
     extern fprintf
     extern malloc
     extern free
     extern fgets
     extern stderr
     extern stdin
     extern stdout
main:
    pushad                ; backup registers
    CALL my_calc          ; call the function my_calc
  returnAddress:
    print_int(eax)        ; print return value(op_count) from EAX
    popad                 ; restore registers
  exit_program:
    mov ebx,0             ; exit program
    mov eax,1
    int 0x80


my_calc:
  init_func
  my_calc_code:
    print_string(CALC_PROMPT)   ; print prompt to user
    get_input                   ; get input from user
    mov al, byte[BUFFER]
    is_end?
    is_add?
    is_pop_and_print?
    is_duplicate?
    is_bitwize_and?
    is_operand?
  jmp my_calc_code
  end_calc:
  mov eax, dword [op_count]     ; return op_count in eax
  return_from_func

create_operand:
  init_func
  count_input_length
  mov ecx, BUFFER           ; store a pointer to the inputs leftmost digit (first char in BUFFER) in ecx
  mov ebx, 0                ; reset ebx to 0, will store the head of the operand list
  test eax, 1               ; check the evanity of the input strings length
  jz .next_link             ; if input length is even start converting input to a list
  mov al, '0'               ; else, input length is odd thus,
  mov ah, byte [ecx]        ; add a leading zero to the first LINK_SIZE
  add_link al, ah           ; represenig the operands msb
  inc ecx                   ; point to next digit (char in BUFFER)
  .next_link:               ; for each pair of chars (at this point there remains an even number of chars to convert)
    mov al, byte [ecx]      ; mov the left digit of the current pair to al
    inc ecx                 ; point to next digit (char in BUFFER)
    cmp al, 10              ; check for end of line (represented by the new line ascii char)
    jz .retrun_op           ; if reached end, return converted oprand list
    mov ah, byte [ecx]      ; else, mov the right digit of the current pair to ah
    inc ecx                 ; point to next digit (char in BUFFER)
    add_link al, ah         ; create a new link with the current pair of digits converted to BCD and add them at the head of the oprand list
    jmp .next_link          ; continue to the next pair of digits
  .retrun_op:
  mov eax, ebx              ; return a pointer the the operand list in eax
  return_from_func

calc_add:
  ;TODO:
  init_func
  mov eax, 0
  return_from_func

calc_pop_and_print:
  init_func
  calc_pop_and_print_code:
    sufficient_args? 2      ; check sufficiant number of args
    pop_op                  ; pop pointer to operand on to eax
    mov ecx, eax            ; move pointer to operand to ecx
    .next_link:
      init_func
      reached_end? ecx        ; jump to end_oeration if we have reached the end of the operand
      pushad
      get_next(ecx)           ; move to next two digits in first operand
      call .next_link
      popad
      get_data(ecx)
      print_two_digits(eax)
    .end_operation:
  return_from_func

calc_duplicate:
 ;TODO:
  init_func
  mov eax, 0
  return_from_func

calc_bitwize_and:
  init_func
  calc_bitwize_and_code:
    sufficient_args? 2      ; check sufficiant number of args
    pop_op                  ; pop pointer to first operand on to eax
    mov ecx, eax            ; move pointer to first operand to ecx
    mov ebx, ecx            ; backuup pointer to head of the first oprand list in ebx
    pop_op                  ; pop pointer to second operand on to eax
    mov ebx, eax            ; move pointer to second operand to ecx
    .next_link:
      reached_end? ecx        ; jump to end_oeration if we have reached the end of the first operand
      reached_end? ebx        ; jump to end_oeration if we have reached the end of the second operand
      get_data(ebx)           ; copy two digits from second oprand to al
      and byte [ecx], al      ; prform bitwize and on two digits from bowth operands and save resault in the first
      get_next(ecx)           ; move to next two digits in first operand
      get_next(ebx)           ; move to next two digits in first operand
    jmp .next_link
    .end_operation:
      push_op ebx             ; push first operand back to op_stack
  return_from_func
