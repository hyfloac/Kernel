[BITS 32]

section .bss
align 4

kernel_stack_bottom: equ $
    resb 16384 ; 16 KB
kernel_stack_top:

