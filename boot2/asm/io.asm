[BITS 32]

global out8
global out16
global out32
global in8
global in16
global in32
global io_wait
global bochs_break

out8: ; void __fastcall out8(u16 port, u8 val)
    mov al, dl
    mov dx, cx
    out dx, al
    ret

out16: ; void __fastcall out16(u16 port, u16 val)
    mov ax, dx
    mov dx, cx
    out dx, ax
    ret

out32: ; void __fastcall out32(u16 port, u32 val)
    mov eax, edx
    mov dx, cx
    out dx, eax
    ret

in8: ; u8 __fastcall in8(u16 port)
    mov dx, cx
    in al, dx
    ret

in16: ; u16 __fastcall in16(u16 port)
    mov dx, cx
    in ax, dx
    ret

in32: ; u32 __fastcall in32(u16 port)
    mov dx, cx
    in eax, dx
    ret

io_wait: ; void io_wait()
    mov dx, 0x0FED
    out dx, al      ; Dump random data to unused port 0x0FED
    ret

bochs_break: ; void bochs_break()
    xchg bx, bx
    ret
