%macro print 2       ; call printf
  pusha
  push dword %1
  push dword %2
  call printf
  add esp, 8
  popa
%endmacro

%macro fprint 3       ; call fprintf
  pusha
  push dword %1
  push dword %2
  push dword [%3]
  call fprintf
  add esp, 12
  popa
%endmacro

%macro print_op 2
  pusha
  push dword %2
  push dword %1
  call print_operand
  add esp, 8
  popa
%endmacro

%macro debug_mode 2
  cmp dword[DEBUGE], 1
  jnz %%.not_debug_mode
  print_err(DEBUG_PROMPT)
  print_err(%1)
  print_op %2, stderr
  print_new_line(stderr)
  %%.not_debug_mode:
%endmacro

%macro init_func 0
  push ebp           ; save previous value of ebp
  mov ebp, esp       ; set ebp to point to myFunc frame
%endmacro

%macro return_from_func 0
  mov esp, ebp      ; "delete" local variables of myFunc
  pop ebp           ; restore previous value of ebp
  RET               ; return to the caller
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
  jnz %%.not_add
    pusha
    call calc_add
    popa
    jmp my_calc_code
  %%.not_add:
%endmacro

%macro is_pop_and_print? 0
  cmp al, 'p'
  jnz %%.not_p
    pusha
    call calc_pop_and_print
    popa
    jmp my_calc_code
  %%.not_p:
%endmacro

%macro is_duplicate? 0
  cmp al, 'd'
  jnz %%.not_add
    pusha
    call calc_duplicate
    popa
    jmp my_calc_code
  %%.not_add:
%endmacro

%macro is_bitwize_and? 0
  cmp al, '&'
  jnz %%.not_and
    pusha
    call calc_bitwize_and
    popa
    jmp my_calc_code
  %%.not_and:
%endmacro

%macro is_debug? 0
  mov eax, dword [ebp+8]        ; get argc
  cmp eax, 2                    ; enogh args?
  jb %%.not_debug
  mov eax, dword [ebp+12]       ; get argv
  mov ebx, dword [eax + 4]      ; get argv[1]
  cmp byte [ebx], '-'           ;check argv[1] = "-d"
  jnz %%.not_debug
  cmp byte [ebx + 1], 'd'
  jnz %%.not_debug
    mov dword [DEBUGE], 1
  %%.not_debug:
%endmacro

%macro is_operand? 0
  mov ecx, 0
  cmp byte [BUFFER+ecx], 10                ; input is not an empty string
  jz %%.illegal
  %%.next_digit:
  is_new_line? byte [BUFFER+ecx], %%.operand_code  ; for each char in the input string
  cmp byte [BUFFER+ecx], '0'                       ; check whether it represents a number between 0-9
  jl %%.illegal                                    ; if not it is an illegal input
  cmp byte [BUFFER+ecx], '9'
  jg  %%.illegal
  inc ecx
  jmp %%.next_digit
  %%.operand_code:
  pushad               ; backup registers
  call create_operand  ; create and operand list from input
  push_op eax          ; push created operand to op_stack
  popad                ; restore registers
  jmp my_calc_code
  %%.illegal:
  print_err(ERR_ILLEGAL_INPUT)
%endmacro

%macro overflow? 0
  cmp dword [stack_counter], STACK_SIZE
  jnz %%.no_overflow
  print_err(ERR_OVERFLOW)
  return_from_func
  %%.no_overflow:
%endmacro

%macro push_op 1
  overflow?
  debug_mode PUSH_PROMPT, %1
  push ecx
  mov ecx, dword [stack_counter]
  mov [OP_STACK+4*ecx], %1
  inc dword [stack_counter]
  pop ecx
%endmacro

%macro sufficient_args? 1
  cmp dword[stack_counter], %1
  jae %%.sufficient_args
  print_err(ERR_NUM_OF_ARGS)
  return_from_func
  %%.sufficient_args:
%endmacro

%macro pop_op 1
  sufficient_args? 1
  push ecx
  mov ecx, dword[stack_counter]
  mov %1, [OP_STACK+4*(ecx-1)]
  dec dword[stack_counter]
  debug_mode POP_PROMPT, %1
  pop ecx
%endmacro

%macro allocate_link 0    ; allocating memory with call to malloc
  pusha
  push LINK_SIZE
  call malloc
  mov [return], eax
  add esp, 4
  popa
%endmacro

%macro free_link 1
  pusha
  push %1
  call free
  add esp, 4
  popa
%endmacro

%macro free_list 1
  is_null? %1, %%.end_free
    mov ecx, %1
    get_next(%1)
    free_link ecx
  %%.end_free:
%endmacro

%macro add_link 3               ; add_link(list,tens,ones) : converts a pair of digits (represented by ascii chars) to a BCD number
  sub %2, '0'                   ; and store them in a link and set it as the head of the list in %1
  shl %2, 4                     ; convert left digit from ascii to BCD
  sub %3, '0'                   ; convert right digit from ascii to BCD
  add %2, %3                    ; join the two in to a single number in BCD
  add_link_to_head %1, %2
%endmacro

%macro add_link_to_head 2       ;add_link_to_head(list,data)
  push ecx
  mov ecx, %1                   ; save pointer to rest of the list in ecx
  allocate_link                 ; allocate space for a link to store this number
  mov %1, dword [return]        ; move pointer to allocated space from return (variable) to %1
  set_data(%1,%2)               ; store number in link
  set_next(%1,ecx)              ; store pointer to the rest of the list in link
  pop ecx
%endmacro

%macro append_link 2            ; append_link(tail, data) : append new link with data to tail
  push ecx
  allocate_link                 ; allocate space for a link to store this number
  mov ecx, dword [return]       ; move pointer to allocated space from return (variable) to edx
  set_data(ecx,%2)              ; store number in link
  set_next(ecx,0)               ; set new tails next to null
  set_next(%1,ecx)              ; append link to tail
  mov %1, ecx
  pop ecx
%endmacro

%macro call_match_lengths 2
  pusha
  push %2
  push %1
  call match_lengths
  add esp, 8
  popa
  mov ecx, dword [temp]
%endmacro

%macro is_null? 2
  cmp %1, 0
  jz %2
%endmacro

%macro is_new_line? 2
  cmp %1, 10
  jz %2
%endmacro

%macro is_not_zero? 2
  cmp %1, '0'
  jnz %2
%endmacro


%define print_int(i) print i, FORMAT_INT
%define print_char(c) print c, FORMAT_CHAR
%define print_string(s) print s, FORMAT_STRING
%define print_err(err) fprint err, FORMAT_STRING, stderr
%define print_first_two_digits(c,fd) fprint c, FORMAT_UNSIGNED_CHAR_NO_ZERO, fd
%define print_two_digits(c,fd) fprint c, FORMAT_UNSIGNED_CHAR, fd
%define print_new_line(fd) fprint NEW_LINE, FORMAT_STRING, fd
%define get_data(link, destination) mov destination, byte [link]
%define get_next(link) mov link, dword [link+1]
%define set_data(link,data) mov byte [link], data
%define set_next(link,next) mov dword [link+1], next

STACK_SIZE equ 5
BUFFER_SIZE equ 82
LINK_SIZE equ 5


section .rodata
FORMAT_INT:
  db "%d", 10,0                                             ; int formating string
FORMAT_CHAR:
  db "%c", 10,0                                             ; char formating string
FORMAT_UNSIGNED_CHAR_NO_ZERO:
  db "%hhx", 0                                              ; formating string for printing left most byte
FORMAT_UNSIGNED_CHAR:
  db "%02hhx", 0                                            ; formating string for printing two digits (byte) in an operand
FORMAT_STRING:
    db "%s", 0                                              ; string formating string
NEW_LINE:
  db 10,0
CALC_PROMPT:
  db "calc: ", 0                                            ; prompt to user
DEBUG_PROMPT:
  db "@ ", 0
PUSH_PROMPT:
  db "pushing operand: ", 0
POP_PROMPT:
  db "poped operand: ", 0
ERR_OVERFLOW:
  db "Error: Operand Stack Overflow", 10, 0                 ; prompt for Operand Stack Overflow
ERR_NUM_OF_ARGS:
  db "Error: Insufficient Number of Arguments on Stack", 10, 0 ; prompt for Insufficient Number of Arguments on Stack
ERR_ILLEGAL_INPUT:
  db "Error: illegal input", 10, 0

section .data
op_count:
  dd 0
stack_counter:
  dd 0
return:
  dd 0
temp:
  dd 0
DEBUGE:
  dd 0

section .bss
OP_STACK:
  resd STACK_SIZE
BUFFER:
  resb BUFFER_SIZE
ARGV:
  resb 3


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
    init_func
    is_debug?
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
  mov eax, 0
  mov ecx, 0
  mov edx, 0
remove_leading_zeros:
    is_not_zero? byte [BUFFER+ecx], count_input_length
    inc ecx
  jmp remove_leading_zeros

count_input_length:
  mov edx, ecx
  is_new_line? byte [BUFFER+ecx], .input_is_zero
  .next_digit:
    is_new_line? byte [BUFFER+ecx], convert_to_bcd_list
    inc ecx
  jmp .next_digit
  .input_is_zero:
    dec edx

convert_to_bcd_list:
  mov ebx, 0                      ; reset ebx to 0, will store the head of the operand list
  mov eax, ecx
  sub eax, edx
  test eax, 1                     ; check the evanity of the input strings length
  jz .next_link                   ; if input length is even start converting input to a list
  mov al, '0'                     ; else, input length is odd thus,
  mov ah, byte [BUFFER+edx]       ; add a leading zero to the first LINK_SIZE
  add_link ebx, al, ah            ; represenig the operands msb
  inc edx                         ; point to next digit (char in BUFFER)
  .next_link:                     ; for each pair of chars (at this point there remains an even number of chars to convert)
      cmp edx, ecx
      je .return_op
      mov al, byte [BUFFER+edx]   ; mov the left digit of the current pair to al
      inc edx                     ; point to next digit (char in BUFFER)
      mov ah, byte [BUFFER+edx]   ; else, mov the right digit of the current pair to ah
      inc edx                     ; point to next digit (char in BUFFER)
      add_link ebx, al, ah        ; create a new link with the current pair of digits converted to BCD and add them at the head of the oprand list
  jmp .next_link                  ; continue to the next pair of digits
  .return_op:
  mov eax, ebx                    ; return a pointer the the operand list in eax
  return_from_func

match_lengths:
  init_func
  mov ebx, dword [ebp+8]
  mov edx, dword [ebp+12]
  mov dword[temp], 0
    .next_link:
        is_null? ebx, .first_is_null
        is_null? edx, .second_is_null
        ; both are not null
        inc dword [temp]
        mov eax, ebx
        get_next(ebx)
        mov ecx, edx
        get_next(edx)
      jmp .next_link
    .first_is_null:
        mov ebx, eax
      .extend_first:
        is_null? edx, .lengths_match
        ;first is null but second isnt
        inc dword [temp]
        append_link ebx, 0
        get_next(edx)
      jmp .extend_first
    .second_is_null:
        mov edx, ecx
      .extend_second:
        is_null? ebx, .lengths_match
        ;second is null but first isnt
        inc dword [temp]
        append_link edx, 0
        get_next(ebx)
      jmp .extend_second
    .lengths_match:
  return_from_func


calc_add:
  init_func
    sufficient_args? 2      ; check sufficiant number of args
    pop_op ebx              ; pop pointer to first operand on to ebx
    pop_op edx              ; pop pointer to second operand on to edx
    call_match_lengths ebx, edx
    push ebx                ; backup head of first operand list
    push edx                ; backup head of second operand list
    .next_link:
      get_data(ebx, al)     ; put two digits from first oerand in al
      get_data(edx, ah)     ; put two digits from second oerand in ah
      adc al, ah            ; add with carry
      daa                   ; adjust to decimal
      set_data(ebx, al)     ; save in first operand
      mov eax, ebx              ; backup pointer to current link first operand in eax
      get_next(ebx)             ; move to next two digits in first operand
      get_next(edx)             ; move to next two digits in second operand
    loop .next_link, ecx
    jnc .end_operation          ; if there is carry left
      mov ebx, eax              ; restore last none null link of first oerand in ebx
      append_link ebx, 1        ; append to the end (msb) alink with the value 1
    .end_operation:
      pop edx
      free_list edx
      pop ebx
      push_op ebx             ; push first operand back to op_stack

      inc dword [op_count]    ; increase opration counter
    return_from_func

calc_pop_and_print:
  init_func
    sufficient_args? 1                ; check sufficiant number of args
    pop_op ebx                        ; pop an operand list to edx
    print_op ebx, stdout              ; print operand to stdout
    print_new_line(stdout)
    inc dword [op_count]              ; increment opration conter
    free_list ebx                     ; free lists allocated memory
  return_from_func

print_operand:
  init_func
    mov ebx, dword [ebp+8]                  ; get pointer to operand list
    mov edx, dword [ebp+12]                 ; get destination file descriptor
    mov ecx, 0                        ; reset  counter with 0 (will count the number of pair digits in the operand)
    .next_link:
      is_null? ebx, .print_list       ; while havent reached the end of the list (NULL pointer)
      get_data(ebx, al)                   ; eax <- current links data (teo digits)
      push eax                        ; push two digits (to later be printed in a reversed order)
      get_next(ebx)                   ; get next link
      inc ecx                         ; increment counter
    jmp .next_link
    .print_list:                      ; print pushed digits
      pop eax
      dec ecx
      print_first_two_digits(eax,edx)
      is_null? ecx, .end_operation
    .next_two_digits:
      pop eax                           ; pop two digits
      print_two_digits(eax,edx)        ; print them
    loop .next_two_digits, ecx        ; continue while counter != 0
    .end_operation:
  return_from_func



calc_duplicate:
  init_func
    sufficient_args? 1            ; check sufficiant number of args
    pop_op edx                    ; pop pointer original operand list on to edx
    push_op edx                   ; return pointer to original to stak (a pointer to it still remains in edx)
    get_data(edx, al)             ; copy two digits from the original operand list to al
    mov ebx, 0
    add_link_to_head ebx, al
    push ebx                  ; edx store tail of the duplicated list
    .next_link:
      get_next(edx)
      is_null? edx, .end_operation
      get_data(edx, al)           ; copy two digits from the original operand list to al
      append_link ebx, al     ; append new link with the next two duguts to the end of the duplicated list
    jmp .next_link
    .end_operation:
      pop ebx
      push_op ebx            ; push duplicated list to stack
      inc dword [op_count]     ; increment operation counter
  return_from_func

calc_bitwize_and:
  init_func
    sufficient_args? 2      ; check sufficiant number of args
    pop_op ebx              ; pop pointer to first operand on to ebx
    pop_op edx              ; pop pointer to second operand on to edx
    call_match_lengths ebx, edx
    push ebx                ; backup pointer to head of the list in order to free its allocated memory later
    mov dword[temp], ebx  ; will store the msb of the operations result or ist lsb if the result is 0, used to clean leading zeros
    push edx                ;backup pointer to head of the list in order to free its allocated memory later
    .next_link:
      get_data(edx, al)
      and byte [ebx], al
      jz .same_msb
        mov dword[temp], ebx  ; result of and was not 0, update the current link to be the new msb of the final result
      .same_msb:
      get_next(ebx)             ; move to next two digits in first operand
      get_next(edx)             ; move to next two digits in second operand
    loop .next_link, ecx
    .end_operation:
      pop edx
      free_list edx
      mov ebx, dword[temp]
      mov edx, ebx
      get_next(edx)           ; delete all leading zeros
      free_list(edx)          ; aka all digits to the left of the msb
      set_next(ebx,0)
      pop ebx                 ; retreve head of first operand
      push_op ebx             ; push final result back to op_stack
      inc dword [op_count]    ; increase opration counter
  return_from_func
