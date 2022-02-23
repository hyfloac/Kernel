[BITS 64]

global check_for_cpuid
global cpuid64

check_for_cpuid: ; bool check_for_cpuid()
    pushfd                      ; Save EFLAGS
    pushfd                      ; Store EFLAGS
    xor dword [esp], 0x00200000 ; Invert the ID bit in EFLAGS
    popfd                       ; Load the EFLAGS (with the inverted ID bit)
    pushfd                      ; Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                     ; eax = modified EFLAGS
    xor eax, [esp]              ; eax = changed bits
    popfd                       ; Restore original flags
    and eax, 0x00200000         ; eax = zero if ID bit can't be changed, else non-zero
    ret

; Get the cpuid information
; Params:
;   u64 code - rdi
;   u64* rax - rsi
;   u64* rbx - rdx
;   u64* rcx - rcx
;   u64* rdx - r8
cpuid: ; void cpuid(u64 code, u64* rax, u64* rbx, u64* rcx, u64* rdx)
    mov r11, rbx        ; Save rbx as it'll be changed by cpuid (rax, rdx, and rax are volatile)
    mov r9, rdx         ; Save u64* rbx
    mov r10, rcx        ; Save u64* rcx
    cpuid
    mov [rsi], rax      ; Store rax
    mov [r9], rbx       ; Store rbx
    mov [r10], rcx      ; Store rcx
    mov [r8], rdx       ; Store rdx
    mov rbx, r11        ; Restore rbx
    ret
