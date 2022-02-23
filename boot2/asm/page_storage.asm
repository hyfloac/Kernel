[BITS 32]

section .bss
align 4096

global page_directory_storage
global page_table_storage

page_directory_storage: equ $
    resb 16384 ; 16 KB

page_table_storage: equ $
    resb 16384 ; 16 KB
