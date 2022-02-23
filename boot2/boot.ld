OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x9D00;

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
