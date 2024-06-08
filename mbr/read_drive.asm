CheckCHSSizes:
    mov dl, BYTE [bootDrive] ; Load drive number
    test dl, dl
    jns .exit
    mov ah, 0x08             ; Check drive geometry
    int 0x13
    jc .exit
    inc dh
    mov BYTE [numberOfHeads], dh
    and cl, 0x3F
    mov BYTE [sectorsPerTrack], cl
.exit:
    ret

ExtensionsSupported:
    mov ah, 0x41              ; Check extensions present
    mov dl, BYTE [bootDrive]  ; Drive to check
    mov bx, 0x55AA            ; Magic
    int 0x13
    ret

ReadSectorLegacy:
    mov edx, ebx         ; Copy EBX to EDX
    shr edx, 16          ; Move the upper bits of EDX to DX
    mov ax, bx           ; Move the lower bits of EBX to AX
    xor bx, bx
    mov bl, BYTE [sectorsPerTrack] ; Load the sector size
    div bx               ; Divide by the sector size
    inc dx               ; Increment the sector since it uses 1 indexing.
    push dx              ; Save sector

    xor dx, dx
    mov bh, 0
    mov bl, BYTE [numberOfHeads] ; Load the number of heads
    div bx               ; Divide by the number of heads

    mov dh, dl           ; Head
    mov bx, ax           ; Save cylinder
    mov al, cl           ; Sector Count
    mov cx, bx           ; Prep Cylinder & Sector
    shl cx, 5            ; Create space for the sector
    pop bx               ; Restore sector
    and bx, 0x3F         ; Isolate the low 6 bits
    or cx, bx            ; Merge the sector into the cylinder 

    mov bx, di           ; Buffer location
    mov ah, 0x02         ; Set read mode
    mov dl, BYTE [bootDrive]         ; 0x80 is the C drive
    int 0x13     
    ret

ReadSectorsLegacy:
    push ebx
    push di
.loop:
    pop di
    pop ebx

    push ebx
    push di

    mov cx, 1                ; Load the sector count
    call ReadSectorLegacy

    jc ERROR                 ; Carry is set on error
    pop di
    pop ebx

    inc ebx
    push ebx
    add edi, 512
    push di

    mov ebx, edi
    shr ebx, 16
    mov es, bx

    dec si
    jnz .loop
    pop di
    pop ebx
    ret

sectorsPerTrack: db 18
numberOfHeads: db 2
