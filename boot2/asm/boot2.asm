[BITS 32]

section .boot

extern kmain                    ; The C Kernel Entry Point

c_bouncepad:
    mov esp, kernel_stack_top   ; Setup the stack for the kernel
    call kmain                  ; Transfer to the C Kernel
halt:
    ; cli                         ; Prevent Interrupts from triggering
.halt0:
    hlt                         ; Loop Halt
    jmp .halt0

%include "memset.asm"
%include "paging.asm"
%include "kernel_stack.asm"
%include "page_storage.asm"
