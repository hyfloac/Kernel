[BITS 16]

global memory_table_size

MAGIC_SMAP equ 0x534D4150       ; The magic number "SMAP"

check_mem:
    push es
    mov ax, 0x4000              ; The table will be at 0x40008, so the segment will be 0x4000
    mov es, ax

    mov di, 0x0008              ; The table will be at 0x40008 for 8 byte alignment

    xor ebx, ebx
    xor bp, bp                  ; The table entry count

    mov edx, MAGIC_SMAP         ; Store magic word "SMAP"
    mov eax, 0xE820             ; 
    mov [es:di + 20], dword 1   ; Force ACPI 3.X entry
    mov ecx, 24                 ; Attempt to get 24 byte entries
    int 0x15

    jc .failed

    mov edx, MAGIC_SMAP         ; Some BIOS's may trash this register.
    cmp eax, edx                ; On success EAX will be set to "SMAP"
    jne .failed

    test ebx, ebx               ; If ebx == 0 there is only 1 entry
    jz .failed

    jmp .jmpin
.e820lp:
    test ebx, ebx               ; If EBX is 0, list is complete
    jz .e820f
    
    mov eax, 0xE820             ; EAX and ECX are volatile for int 0x15
    mov [es:di + 20], dword 1   ; Force ACPI 3.X Entry
    mov ecx, 24                 ; Attempt to get 24 byte entries
    int 0x15

    jc .e820f                   ; Carry set means "end of list already reached"

    mov edx, MAGIC_SMAP         ; Some BIOS's may trash this register
.jmpin:
    jcxz .e820lp                ; Skip 0 length entries
    cmp cl, 24                  ; Was the entry 24 bytes
    je .ext_ent
.handle_ent:
    mov ecx, [es:di +  8]       ; Get lower u32 of memory region length
    or  ecx, [es:di + 12]       ; Or it is with the upper 32 bits to test for zero
    jz .e820lp
    inc bp                      ; Increment valid entry count
    add di, 24
    jmp .e820lp
.ext_ent:
    test byte [es:di + 20], 1   ; Should this entry be ignored?
    jne .handle_ent
    jmp .e820lp
.e820f:
    mov [es:0x0], bp            ; Store the table count at 0x40000
    clc                         ; Success
    pop es
    ret
.failed:
    mov [es:0x0], word 0xFFFF
    stc                         ; Function unsupported
    pop es
    ret
