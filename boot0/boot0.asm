[BITS 16]
[ORG 0x7C00]

; FAT BIOS Parameter Block
jmp short kernel_entry   ; Make a relative short jump to the actual bootloader, this is part of the FAT header.
nop                      ; This is part of the FAT header. Required for computing the short jump.
db "MSWIN4.1"            ; OEM Identifer
dw 0x0200                ; Bytes per Sector
db 0x01                  ; Sectors per Cluster
dw 0x0020                ; Reserved Sector Count
db 0x02                  ; File Allocation Table Count
dw 0x0000                ; Directory Count
dw 0x0000                ; Total Sectors in Logical Volume
db 0xF8                  ; Media Descriptor (Fixed Disk)
dw 0x0000                ; Logical Sectors per File Allocation Table
dw 0x0000                ; Number of sides of storage media
dd 0x00000000            ; Number of hidden sectors
dd 0x00000000            ; Large Sector Count

; FAT Extended BIOS Parameter Block
dd 0x00000000            ; Sectors per File Access Table
dw 0x0000                ; Flags
dw 0x0000                ; FAT Version
dd 0x00000002            ; Cluster Number of Root Directory
dw 0x0000                ; Sector Number of FSInfo Structure
dw 0x0000                ; Sector Number of Backup Boot Structure
times 12 db 0x0          ; 12 Reserved Bytes
db 0x80                  ; Drive Number (C Drive)
db 0x00                  ; Windows NT Flags (Reserved)
db 0x28                  ; Signature
dd 0x00000000            ; Volume ID
db "Kernel     "         ; Volume Label (11 Characters, Padded with Spaces)
db "FAT32   "            ; System Identifer String, Always "FAT32   ", Ignore Value

kernel_entry:
jmp 0:boot               ; Force the processor to be in segment 0 while executing
     
boot:    
    xor ax, ax           ; Setup segment registers
    mov ds, ax   
    mov ss, ax   
    mov sp, 0x9C00       ; Setup stack pointer
         
    cld                  ; Clear the direction flag
    cli                  ; Clear the interrupt flag

    mov si, Boot1_DAPACK ; Address of the Boot1 Disk Address Packet
    mov ah, 0x42         ; Set read mode
    mov dl, 0x80         ; 0x80 is the C drive
    int 0x13     

    mov si, Boot2_DAPACK ; Address of the Boot2 Disk Address Packet
    mov ah, 0x42         ; Set read mode
    mov dl, 0x80         ; 0x80 is the C drive
    int 0x13     

    jc .error            ; Carry is set on error
    jmp 0:0x0500         ; Jump to the extended 16-Bit bootloader.
.error:  
    hlt                  ; Halt processor execution
    jmp .error           ; In case we get an interrupt jump back to halt

Boot1_DAPACK:            ; The boot1 data
         db 0x10     
         db 0x00     
.blkcnt: dw 8            ; INT 0x13 will set this to the number of blocks read
.db_add: dw 0x0500       ; Memory buffer desintation address
         dw 0x00         ; Page 0
.d_lba:  dd 0x01         ; Put the LBA to read in this spot
         dd 0x00         ; More storage bytes only for big LBA's
     
Boot2_DAPACK:            ; The boot2 data
         db 0x10     
         db 0x00     
.blkcnt: dw 64           ; INT 0x13 will set this to the number of blocks read
.db_add: dw 0x9D00       ; Memory buffer desintation address
         dw 0x00         ; Page 0
.d_lba:  dd 0x09         ; Put the LBA to read in this spot
         dd 0x00         ; More storage bytes only for big LBA's

times 510-($-$$) db 0
db 0x55
db 0xAA
