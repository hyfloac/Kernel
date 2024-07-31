[BITS 32]

section .text.prologue

extern kmain

global _start

_start:
    jmp kmain

