gdt_start:
    dq 0
gdt_code:
    dw 0xFFFF       ; Limit Low
    dw 0x0000       ; Base Low
    db 0x00         ; Base Middle
    db 10011010b    ; Access Byte 
                    ;   [Pr] Present - 1 (Valid)
                    ;   [Privl] Privilege - 00 (Ring 0)
                    ;   [S] Descriptor Type - 1 (Code/Segment)
                    ;   [Ex] Executable - 1 (Executable)
                    ;   [DC] Conforming - 0 (Requires Ring 0)
                    ;   [RW] Read/Write - 1 (Readable)
                    ;   [Ac] Accessed - 0 (Set when the CPU accesses this segment)
    db 11001111b    ; Flags & Limit High
                    ;   [Gr] Granularity - 1 (4 KiB Blocks)
                    ;   [Sz] Size - 1 (32 Bit Protected)
                    ;   [0] - 0
                    ;   [0] - 0
                    ;    Limit High: 0xF
    db 0x00         ; Base High
gdt_data:
    dw 0xFFFF       ; Limit Low
    dw 0x0000       ; Base Low
    db 0x00         ; Base Middle
    db 10010010b    ; Access Byte 
                    ;   [Pr] Present - 1 (Valid)
                    ;   [Privl] Privilege - 00 (Ring 0)
                    ;   [S] Descriptor Type - 1 (Code/Segment)
                    ;   [Ex] Executable - 0 (Data)
                    ;   [DC] Conforming - 0 (Requires Ring 0)
                    ;   [RW] Read/Write - 1 (Writable)
                    ;   [Ac] Accessed - 0 (Set when the CPU accesses this segment)
    db 11001111b    ; Flags & Limit High
                    ;   [Gr] Granularity - 1 (4 KiB Blocks)
                    ;   [Sz] Size - 1 (32 Bit Protected)
                    ;   [0] - 0
                    ;   [0] - 0
                    ;    Limit High: 0xF
    db 0x00         ; Base High
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start
    dd gdt_start
    dw 0x0000
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

code_seg: dd CODE_SEG
data_seg: dd DATA_SEG

global code_seg
global data_seg
