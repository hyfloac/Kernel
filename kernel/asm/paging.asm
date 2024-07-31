[BITS 32]

section .text

global enable_pae32
global set_cr3_page_pointer32
global flush_tlb32

enable_pae32:
    mov eax, cr4
    or eax, 00100000b ; Set PAE bit
    mov cr4, eax
    ret
 
;   Set the 32 bit pointer for either the Page Directory or the Page 
; Directory Pointer Table dependent on whether PAE is enabled.
set_cr3_page_pointer32: ; void __fastcall set_cr3_page_pointer32(void*);
    mov cr3, ecx        ; Move the first parameter into CR3

    mov eax, cr0
    or eax, 0x80000001  ; Set the Paging (PG) and Protection (PE) bits on CR0
    mov cr0, eax
    ret

flush_tlb32:
    mov eax, cr3
    mov cr3, eax
    ret

[BITS 64]
; [BITS 32]

global enable_pae64
global set_cr3_page_pointer64
global flush_tlb64

enable_pae64:
    mov rax, cr4
    or rax, 00100000b ; Set PAE bit
    mov cr4, rax
    ret
 
;   Set the 64 bit pointer for either the Page Directory or the Page 
; Directory Pointer Table dependent on whether PAE is enabled.
set_cr3_page_pointer64: ; void set_cr3_page_pointer(void*);
    mov cr3, rdi        ; Move the first parameter into CR3

    mov rax, cr0
    mov rdx, 0x0000000080000001 ; Fix warning aobut DWORD exceeding bounds
    or rax, rdx         ; Set the Paging (PG) and Protection (PE) bits on CR0
    mov cr0, rax
    ret

flush_tlb64:
    mov rax, cr3
    mov cr3, rax
    ret
