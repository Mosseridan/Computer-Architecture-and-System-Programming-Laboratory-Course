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

%macro allocate_link 0                                     ; allocating memory with call to malloc
  pushad
  push LINK_SIZE
  call malloc
  mov [return], eax
  add esp, 4
  popad
%endmacro

%macro get_input 0
  push dword [stdin]
  push BUFFER_SIZE
  push BUFFER
  call fgets
  add esp, 12
%endmacro

%macro is_end? 0
  cmp byte [BUFFER], 'q'
  jnz my_calc_code
%endmacro

%macro push_op 1
  overflow?
  mov ecx, dword [stack_counter]
  mov [OP_STACK+ecx], %1
  inc dword [stack_counter]
%endmacro

%macro overflow? 0
  cmp dword [stack_counter], STACK_SIZE
  jnz .no_overflow
  print_err(ERR_OVERFLOW)
  jmp my_calc_code
  .no_overflow:
%endmacro

%macro pop_op 0
  empty_stack?
  mov ecx, dword[stack_counter]
  mov eax, [OP_STACK+ecx]
  dec dword[stack_counter]
%endmacro

%macro empty_stack? 0
  cmp dword[stack_counter], 0
  jnz .stack_not_empty
  print_err(ERR_NUM_OF_ARGS)
  jmp my_calc_code
  .stack_not_empty:
%endmacro



%macro set_calc 0                                         ; check if operand is zero and if its and
  cmp [bl], 0
  jz check_cl                                             ; if first operand is 0- check the second operand
  cmp [cl], 0
  jz cl_is_zero_bl_not
  jmp end_set_calc
check_cl:                                                 ; if second operand is 0- end
 cmp [cl], 0
 jnz bl_is_zero_cl_not
 push_op(eax)
 jmp my_calc_code
bl_is_zero_cl_not:                                        ; flag- bl is zero
  mov bh, 1
  jmp end_set_calc
cl_is_zero_bl_not:                                        ; flag- cl is zero
  mov ch, 1
end_set_calc:
%endmacro

%macro create_operand 0
  count_input_length
  mov ecx, BUFFER
  mov ebx, 0
  test eax, 1								                               ; check the evanity of the input strings length
  jz .next_link
  mov al, '0'
  mov ah, byte [ecx]
  add_link al, ah
  inc ecx
  .next_link:
    mov al, byte [ecx]
    inc ecx
    cmp al, 10
    jz .retrun_op
    mov ah, byte [ecx]
    inc ecx
    add_link al, ah
    jmp .next_link
  .retrun_op:
  mov eax, ebx
%endmacro

%macro count_input_length 0
  mov ecx, BUFFER
  mov eax, 0
  .next_digit:			                                     ; counts the length of the input string
  cmp byte [ecx], 10			                               ; check for NULL terminator character
  jz .reached_end                                        ; if end of string hasent been reached, continue counting
  inc eax 	 							                               ; increment counter
  inc ecx								                                 ; increment to point at the next char in the input string
  jmp .next_digit
  .reached_end:
%endmacro

%macro add_link 2
  sub %1, '0'
  shl %1, 4
  sub %2, '0'
  add %1, %2
  allocate_link
  mov edx, dword[return]
  mov byte [edx], %1
  mov dword [edx+1], ebx
  mov ebx, edx
%endmacro

%define print_int(i) print i, FORMAT_INT
%define print_char(c) print c, FORMAT_CHAR
%define print_string(s) print s, FORMAT_STRING
%define print_err(err) print_err err, FORMAT_STRING

STACK_SIZE equ 5
BUFFER_SIZE equ 80
LINK_SIZE equ 5

section .rodata
FORMAT_INT:
  db "%d", 10,0                                             ; int formating string
FORMAT_CHAR:
  db "%c", 10,0                                             ; char formating string
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
    push ebp                    ; save previous value of ebp
    mov ebp, esp                ; set ebp to point to myFunc frame
  my_calc_code:
    print_string(CALC_PROMPT)   ; print prompt to user
    get_input                   ; get input from user
    mov ecx, BUFFER             ; take the first char in BUFFER for check
    input_check(ecx)             ; check the input- operator/ operand
    cmp eax, 1                  ; if input is not an operand- jump to end
    jnz end
    create_operand
    push_op(eax)                    ; else push input(operand) into OP_STACK
    pop_op
  end:
    is_end?
    mov eax, dword [op_count]     ; return op_count in eax
  returnFrom_my_calc:
    mov esp, ebp ; "delete" local variables of myFunc
    pop ebp ; restore previous value of ebp
    RET ; return to the caller

    
