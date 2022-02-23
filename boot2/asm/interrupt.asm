[BITS 32]

section .text

global isr_stub_table32
global load_idt32

extern isr_exception_handler
extern isr_handler
extern ConWriteString
global i_print

isr_exception_bouncepad:
    push ebp
    mov ebp, esp

    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi

    mov eax, cr0
    push eax
    mov eax, cr2
    push eax
    mov eax, cr3
    push eax
    mov eax, cr4
    push eax

    mov ax, ds
    push eax
    push dword 0
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    lea ecx, [esp + 0x8]
    call isr_exception_handler

    pop eax
    pop eax
    mov ds, ax
    mov es, ax

    pop eax
    mov cr4, eax
    pop eax
    mov cr3, eax
    pop eax
    mov cr2, eax
    pop eax
    mov cr0, eax

    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    pop ebp
    add esp, 0x8

    cli
.halt:
    hlt
    jmp .halt
    iret

isr_bouncepad: ; Don't need to save CS, EIP, or EFlags
    push ebp        ; Stack frame
    mov ebp, esp
    
    push eax        ; Save General Purpose Registers
    push ebx
    push ecx
    push edx
    push esi
    push edi

    mov ax, ds      ; Save Segment Rgisters
    push ax
    mov ax, gs
    push ax
    mov ax, es
    push ax
    mov ax, ss
    push ax        

    mov ax, 0x10    ; Set Segment Registers
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov ecx, [esp + 36]
    call isr_handler ; Call C Level Handler

    pop ax          ; Restore Segment Registers
    mov ss, ax
    pop ax
    mov es, ax
    pop ax
    mov gs, ax
    pop ax
    mov ds, ax

    pop edi         ; Restore General Purpose Registers
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    pop ebp
    add esp, 4      ; Pop Interrupt number

    iret

%macro isr_err_stub 1
isr_stub32_%+%1:
    push %1
    jmp isr_exception_bouncepad
%endmacro

%macro isr_no_err_stub 1
isr_stub32_%+%1:
    push dword 0
    push %1
    jmp isr_exception_bouncepad
%endmacro

%macro isr_stub 1
isr_stub32_%+%1:
    push %1
    jmp isr_bouncepad
%endmacro

%include "interrupt_stubs.asm"

isr_stub_table32:
%assign i 0
%rep 48
    dd isr_stub32_%+i
%assign i i+1
%endrep

i_print:
    push eax
    push ecx
    push edx
    call ConWriteString
    pop edx
    pop ecx
    pop eax
    iret

load_idt32: ; void __fastcall load_idt32(IDTR32*)
    lidt [ecx]
    sti
    ret

; [BITS 64]

; global isr_stub_table64
; global load_idt64

; %unmacro isr_err_stub 1
; %unmacro isr_no_err_stub 1

; %macro isr_err_stub 1
; isr_stub64_%+%1:
;     call exception_handler
;     iretq
; %endmacro

; %macro isr_no_err_stub 1
; isr_stub64_%+%1:
;     call exception_handler
;     iretq
; %endmacro

; %include "interrupt_stubs.asm"

; isr_stub_table64:
; %assign i 0
; %rep 32
;     dq isr_stub64_%+i
; %assign i i+1
; %endrep

; load_idt64: ; void load_idt64(IDTR64*)
;     lidt [rdi]
;     sti
