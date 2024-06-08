[BITS 16]
[ORG 0x0500]

init:
    call enable_a20
    call check_mem

    mov ax, 0x03
    int 0x10                ; Set VGA to Text Mode 3

    cli

    lgdt [gdt_ptr]

    mov eax, cr0
    or eax, 0x01            ; Set the Protected Mode bit for CR0
    mov cr0, eax
    jmp CODE_SEG:boot2      ; Long jump to the 32 Bit code
    
%include "a20.asm"
%include "gdt.asm"
%include "check_mem.asm"

[BITS 32]

boot2:
    mov ax, DATA_SEG        ; Move the Data Segment into all of the segment registers
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x00009D00          ; Transfer to the C Kernel Bouncepad
