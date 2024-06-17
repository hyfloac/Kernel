OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x9C00;

    .text : 
    {
        *(.boot)
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
}
