[BITS 16]

enable_a20:
    call check_a20
    cmp ax, 1
    mov bx, 1
    je .finish

    call enable_a20_bios
    cmp ax, 0
    mov ax, 1
    mov bx, 2
    je .finish

    call enable_a20_keyboard
    call check_a20
    cmp ax, 1
    mov bx, 3
    je .finish

    call enable_a20_fast
    call check_a20
    cmp ax, 1
    mov bx, 4
    je .finish
    xor ax, ax
    xor bx, bx
.finish:
    ret

check_a20:
    pushf
    push ds
    push es
    push di
    push si

    cli

    xor ax, ax
    mov es, ax

    not ax
    mov ds, ax

    mov di, 0x0500
    mov si, 0x0510

    mov al, byte [es:di]
    push ax

    mov al, byte [ds:si]
    push ax

    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF

    cmp byte [es:di], 0xFF

    pop ax
    mov byte [ds:si], al
    
    pop ax
    mov byte [es:di], al

    mov ax, 0
    je .exit

    mov ax, 1
.exit:
    pop si
    pop di
    pop es
    pop ds
    popf

    ret

enable_a20_bios:
    mov ax, 0x2403          ; A20 Get Support
    int 0x15
    jb .a20_not_supported   ; INT 0x15 is not supported
    cmp ah, 0
    jnz .a20_not_supported  ; INT 0x15 is not supported

    mov ax, 0x2402          ; A20 Get Status
    int 0x15
    jb .a20_failed          ; Couldn't get A20 status
    cmp ah, 0
    jnz .a20_failed         ; Couldn't get A20 status

    cmp al, 1
    jz .a20_activated       ; A20 is already active

    mov ax, 0x2401          ; A20 Get Status
    int 0x15
    jb .a20_failed          ; Couldn't activate A20
    cmp ah, 0
    jnz .a20_failed         ; Couldn't activate A20
.a20_not_supported:
    mov ax, 0x01
    ret
.a20_failed:
    mov ax, 0x02
    ret
.a20_activated:
    xor ax, ax
    ret

enable_a20_keyboard:
    cli

    call .a20_kb_wait
    mov al, 0xAD
    out 0x64, al

    call .a20_kb_wait
    mov al, 0xD0
    out 0x64, al

    call .a20_kb_wait2
    in al, 0x60
    push eax

    call .a20_kb_wait
    mov al, 0xD1
    out 0x64, al

    call .a20_kb_wait
    pop eax
    or al, 2
    out 0x60, al

    call .a20_kb_wait
    mov al, 0xAE
    out 0x64, al

    call .a20_kb_wait
    sti
    ret
.a20_kb_wait:
    in al, 0x64
    test al, 2
    jnz .a20_kb_wait
    ret
.a20_kb_wait2:
    in al, 0x64
    test al, 1
    jz .a20_kb_wait2
    ret

enable_a20_fast:
    in al, 0x92
    test al, 2
    jnz .a20_fast_ret
    or al, 2
    and al, 0xFE
    out 0x92, al
.a20_fast_ret:
    ret
