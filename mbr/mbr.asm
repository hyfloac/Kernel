[BITS 16]
[ORG 0x0500]
 
start:
  cli                         ; We do not want to be interrupted
  xor ax, ax                  ; 0 AX
  mov ds, ax                  ; Set Data Segment to 0
  mov es, ax                  ; Set Extra Segment to 0
  mov ss, ax                  ; Set Stack Segment to 0
  mov sp, ax                  ; Set Stack Pointer to 0
  .CopyLower:
    mov cx, 0x0200            ; 256 WORDs in MBR
    mov si, 0x7C00            ; Current MBR Address
    mov di, 0x0500            ; New MBR Address
    rep movsw                 ; Copy MBR
  jmp 0:LowStart              ; Jump to new Address
 
LowStart:
  sti                         ; Start interrupts
  mov BYTE [bootDrive], dl    ; Save BootDrive
  .CheckPartitions:           ; Check Partition Table For Bootable Partition
    mov bx, PT1               ; Base = Partition Table Entry 1
    mov cx, 4                 ; There are 4 Partition Table Entries
    .CKPTloop:
      mov al, BYTE [bx]       ; Get Boot indicator bit flag
      test al, 0x80           ; Check For Active Bit
      jnz .CKPTFound          ; We Found an Active Partition
      add bx, 0x10            ; Partition Table Entry is 16 Bytes
      dec cx                  ; Decrement Counter
      jnz .CKPTloop           ; Loop
    jmp ERROR                 ; ERROR!
    .CKPTFound:
      mov WORD [PToff], bx    ; Save Offset
      add bx, 8               ; Increment Base to LBA Address
  .ReadVBR:
    push bx
    call CheckCHSSizes
    call ExtensionsSupported
    pop bx
    mov ebx, DWORD [bx]       ; Start LBA of Active Partition
    mov di, 0x7C00            ; We Are Loading VBR to 0x07C0:0x0000
    mov cx, 1                 ; Only one sector
    jc .ReadLegacy
    call ReadSectorsExtended      ; Read Sector
    jmp .jumpToVBR
  .ReadLegacy:
    call ReadSectorLegacy

  .jumpToVBR:
    cmp WORD [0x7DFE], 0xAA55 ; Check Boot Signature
    jne ERROR                 ; Error if not Boot Signature
    mov si, WORD [PToff]      ; Set DS:SI to Partition Table Entry
    mov dl, BYTE [bootDrive]  ; Set DL to Drive Number
    jmp 0x7C00                ; Jump To VBR

ERROR:
    hlt                  ; Halt processor execution
    jmp ERROR            ; In case we get an interrupt jump back to halt

%include "read_drive.asm"

ReadSectorsExtended:
    mov [blkcnt], cx
    mov [db_add], di
    mov [d_lba], ebx
    mov si, Boot_DAPACK  ; Address of the Boot1 Disk Address Packet
    mov ah, 0x42         ; Set read mode
    mov dl, BYTE [bootDrive]         ; 0x80 is the C drive
    int 0x13     
    ret

Boot_DAPACK:             ; The boot1 data
         db 0x10     
         db 0x00     
blkcnt:  dw 1            ; INT 0x13 will set this to the number of blocks read
db_add:  dw 0x7C00       ; Memory buffer desintation address
         dw 0x00         ; Page 0
d_lba:   dd 0x01         ; Put the LBA to read in this spot
         dd 0x00         ; More storage bytes only for big LBA's

; times (218 - ($-$$)) nop      ; Pad for disk time stamp
 
DiskTimeStamp times 8 db 0    ; Disk Time Stamp
 
bootDrive db 0                ; Our Drive Number Variable
PToff dw 0                    ; Our Partition Table Entry Offset
 
times (512 - 72 - ($-$$)) nop    ; Pad For MBR Partition Table
 
dd 0x48794F53 ; MBR - Optional Disk ID
dw 0x0000     ; MBR - Reserved

; Partition Entry 0
PT1:
db 0x80 ; Drive Attributes, Active
db 0xFF ; Start Head
db 0xFF ; Start Sector / Cylinder
db 0xFF ; Start Cylinder
db 0x0B ; System ID, FAT32
db 0xFF ; End Head
db 0xFF ; End Sector / Cylinder
db 0xFF ; End Cylinder
dd 0x00000001 ; Starting LBA
dd 0x00000400 ; Total sectors
; PT1 times 16 db 0             ; First Partition Entry
PT2 times 16 db 0             ; Second Partition Entry
PT3 times 16 db 0             ; Third Partition Entry
PT4 times 16 db 0             ; Fourth Partition Entry
 
dw 0xAA55                     ; Boot Signature