section .rodata
LC0:
  DB "%llX", 10, 0         ; format unsigned long long hex

section .bss
LC1:
  RESB 8
ANS:
  RESB 1

section .text
  align 16
  global calc_func
  extern printf
  extern compare

calc_func:
  push ebp
  mov ebp, esp             ; Entry code - set up ebp and esp
  mov ebx, dword [ebp+8]   ; Get argument (unsigned long long)
  mov ecx, dword[ebp+12]   ; get argument (int numOfRounds)
  pushad                   ; Save registers

  ;mov [numOfRounds], edx    ; keep number of rounds in memory
  ;mov ebx, ecx              ; point to X with ebx

next_round:
 cmp ecx, 0
 jz end
 push ecx                   ; save number of rounds
 mov ecx, 8                 ; initialize counter with 8 (the number of pair digits in x)
 mov eax, 0                 ; initialize eax
.next_two:                  ; iterate through all digits of X in pairs
  dec ecx                   ; decriese counter

  mov al, [ebx+ecx]         ; move the next byte X to al
  shr al, 4                 ; extract the left digit in al
  mov dh, 1                 ;

  .apply_to_y:
    mov dl, 16              ; initialize dl with 16
    test al, 1					   	; check the evanity of the input strings length
    jz .even                ; if even we will icrese the current_digit/2 pair(byte) in Y by 16
    mov dl, 1               ; else we will increase the current_digit-1/2 pair(byte) in Y by 1
    dec al                  ; current_digit <- current_digit-1
    .even:
    shr al, 1               ; devide al by 2 (we should increase the value in the current_digit/2 pair)
    neg al                  ; from the left
    add al, 7               ; applying to [LC1+7-al]
    add byte [LC1+eax], dl  ; increment in y
    cmp dh, 0               ; if this was the second digit of the pair continue to the next pair
    jz .pair_done

    mov al, [ebx+ecx]          ; move the same byte X to al
    and al, 15                 ; extarct the right digit of the two
    mov dh, 0                  ; indicate that this is the seond and last digit of the pair
    jmp .apply_to_y

 .pair_done:
  inc ecx
  loop .next_two, ecx          ; decrement counter and loop back if not 0
 pushad                        ; save registers
 pushfd                        ; save flags
 push LC1                      ; push first argument to compare Function
 push ebx                      ; push second argument to compare Function
 call compare                  ; call compare
 mov [ANS], eax                ; retrieve return value from eax
 add esp, 8                    ; "delete" function arguments (in our case argumentsSize = 2 * dword = 8 bytes)
 popfd                         ; restore flags register
 popad                         ; restore registers

 mov edx, [LC1]               ; move Y to X: move bits 0-31 (32 bits) of Y (out of 64) to temp register edx
 mov [ebx], edx               ; move the the content of edx to the 0-31 bits of X (out of 64)
 mov edx, [LC1+4]             ; move bits 32-63 (32 bits) of Y (out of 64) to temp register edx
 mov [ebx+4], edx             ; move the the content of edx to the 32-64 bits of X (out of 64)

 mov dword [LC1], 0           ; reset Y to 0: bits 0-31
 mov dword [LC1+4], 0         ; reset Y to 0: bits 32-63

 pop ecx                      ; restore number of rounds to ecx
 dec ecx                      ; decrement number of rounds

 cmp byte [ANS], 0            ; if the call to compare returned false (eax = 0)
 jz next_round                ; continue to next round

end:
  push dword [ebx+4]		      ; Call printf with LC1
  push dword [ebx]
  push LC0
  call	printf
  add esp, 16		             ; Clean up stack after call
  popad		                   ; Restore registers
  mov	esp, ebp               ; Function exit code
  pop	ebp
  ret
