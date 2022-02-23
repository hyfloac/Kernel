[BITS 32]

section .text

global memset_stosb
global zeromem_stosb

memset_stosb: ; void __fastcall memset_stosb(void* dest, u8 value, u32 count)
    mov edi, ecx ; Dest
    mov al, dl   ; Value
    pop ecx      ; Count
    rep stosb    ; Set bytes
    ret

zeromem_stosb: ; void __fastcall zeromem_stosb(void* dest, u32 count)
    mov edi, ecx ; Dest
    xor al, al   ; Value
    mov ecx, edx ; Count
    rep stosb    ; Set bytes
    ret
