# Kernel

This is currently a 32 Bit Protected Mode kernel, with the intent of moving to 64 Bit Long Mode in the future.

## Building

This is designed to be built from within Docker. Additionally it is recommended to have the docker plugin for VS code. Once running inside docker simply run `make`.

## Running

To run you will need to be outside docker. This is because the docker image doesn't support graphics, something which QEMU requires. With qemu run the command `qemu-system-i386 -drive file=bootloader.img,format=raw,index=0,media=disk`. Trying `qemu-system-i386 -drive file=bootloader.img,format=raw,index=0,media=disk -serial stdio` for serial.

## Layout

The code is currently split into three parts: `boot0`, `boot1`, and `boot2`. 

`boot0` is the initial entrypoint from the BIOS. It is very small and simply loads in `boot1` from disk to `0x0500` and `boot2` from disk to `0x9D00`, then jumps to `boot1`. 

`boot1` handles the change to 32 Bit Protected Mode. This includes enabling the A20 line, saves the memory map in 16 Bit Real Mode, setting up the GDT, and finally enabling Bit 0 of CR0 (enable 32 Bit Protected Mode). From there a long jump is performed to the 32 bit code which initializes all of the segment registers, finishing with a jump to `boot2` at `0x9D00`.

`boot2` is the primary bootloader that handles the transition to 64 Bit Long Mode. Currently it initializes a backbuffer for the console, remaps the PIC, loads the IDT, and filters the memory map retrieved from the BIOS in `boot1`. It also dumps the filtered memory map and begins the process of slicing chunks of pages for a slab allocator. Currently paging is not enabled as the virtual page manager is not yet designed, and PAE considersations have not been fully undertaken yet.

