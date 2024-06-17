# Memory Map

| Start      | End        | Size       | Description               |
| ---------- | ---------- | ---------- | ------------------------- |
| 0x00000500 | 0x000006FF | 512 Bytes  | Boot 1                    |
| 0x00007C00 | 0x00007DFF | 512 Bytes  | Boot 0                    |
| 0x00007F00 | 0x00009BFF | 7424 Bytes | Boot 0 Stack              |
| 0x00009C00 | 0x00011C00 | 32 KiB     | Protected Mode Bootloader |
| 0x0004F060 | 0x0004FFFF | 4000 Bytes | Video Back Buffer         |
| 0x00050000 | 0x0007FFFF | 192 KiB    | Page Linked List Arena    |

