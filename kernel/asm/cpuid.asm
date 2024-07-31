; [BITS 64]
[BITS 32]

global check_for_cpuid
global cpuid

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
;   u32 code - stack[16]
;   u32* eax - stack[12]
;   u32* ebx - stack[8]
;   u32* ecx - stack[4]
;   u32* edx - stack[0]
cpuid: ; void __stdcall cpuid(u32 code, u32* eax, u32* ebx, u32* ecx, u32* edx)
    push ebx            ; Save ebx as it'll be changed by cpuid (eax, edx, and ecx are volatile)
    mov eax, [esp + 4]  ; Move the code into eax
    cpuid
    push edi            ; Save edi because we'll need it for the pointers
    mov edi, [esp + 12] ; Retrieve u32* edx (stack params are push RTL)
    mov [edi], edx      ; Store edx
    mov edi, [esp + 16] ; Retrieve u32* ecx 
    mov [edi], ecx      ; Store ecx
    mov edi, [esp + 20] ; Retrieve u32* ebx 
    mov [edi], ebx      ; Store ebx
    mov edi, [esp + 24] ; Retrieve u32* eax 
    mov [edi], eax      ; Store eax
    pop edi             ; Restore edi
    pop ebx             ; Restore ebx
    ret 20              ; Clean up the stack

[BITS 64]

global cpuid64

; Get the cpuid information
; Params:
;   u64 code - rdi
;   u64* rax - rsi
;   u64* rbx - rdx
;   u64* rcx - rcx
;   u64* rdx - r8
cpuid64: ; void cpuid(u64 code, u64* rax, u64* rbx, u64* rcx, u64* rdx)
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
