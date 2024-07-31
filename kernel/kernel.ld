OUTPUT_FORMAT("elf32-i386")

ENTRY(_start)

SECTIONS
{
    . = 0x20000;

    .text : 
    {
        *(.text.prologue)
        *(.text)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        __bss_start = .;
        *(COMMON);
        *(.bss);
        __bss_end = .;
    }

    __bss_size = ((__bss_end) - (__bss_start));

    /DISCARD/ :
    {
        *(.note.gnu.build-id)
    }
}
