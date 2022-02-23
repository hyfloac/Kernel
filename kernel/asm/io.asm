[BITS 64]

global out8
global out16
global out32
global in8
global in16
global in32
global io_wait

out8: ; void out8(u16 port, u8 val)
    mov dx, di
    mov al, sil
    out dx, al
    ret

out16: ; void out16(u16 port, u8 val)
    mov dx, di
    mov ax, si
    out dx, ax
    ret

out32: ; void out32(u16 port, u8 val)
    mov dx, di
    mov eax, esi
    out dx, eax
    ret

in8: ; u8 __fastcall in8(u16 port)
    mov dx, di
    in al, dx
    ret

in16: ; u16 __fastcall in16(u16 port)
    mov dx, di
    in ax, dx
    ret

in32: ; u32 __fastcall in32(u16 port)
    mov dx, di
    in eax, dx
    ret

io_wait: ; void io_wait()
    mov dx, 0x0FED
    out dx, al      ; Dump random data to unused port 0xFEED
    ret
