[BITS 64]

global enable_pae
global set_cr3_page_pointer
global flush_tlb

enable_pae:
    mov rax, cr4
    or rax, 00100000b ; Set PAE bit
    mov cr4, rax
    ret
 
;   Set the 64 bit pointer for either the Page Directory or the Page 
; Directory Pointer Table dependent on whether PAE is enabled.
set_cr3_page_pointer: ; void set_cr3_page_pointer(void*);
    mov cr3, rdi        ; Move the first parameter into CR3

    mov rax, cr0
    mov rdx, 0x0000000080000001 ; Fix warning aobut DWORD exceeding bounds
    or rax, rdx         ; Set the Paging (PG) and Protection (PE) bits on CR0
    mov cr0, rax
    ret

flush_tlb:
    mov rax, cr3
    mov cr3, rax
    ret
