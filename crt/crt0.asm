[BITS 64]

extern initialize_standard_library
extern _init
extern main
extern exit

section .text
global _start

_start:
    ; Setup the end of the strack frame linked list.
    xor rbp, rbp
    push rbp
    push rbp
    mov rbp, rsp

    push rsi
    push rdi

    call initialize_standard_library

    call _init

    pop rdi
    pop rsi

    call main

    mov edi, eax
    call exit

.size dd _start, $ - _start


