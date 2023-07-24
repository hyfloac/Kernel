NASM := nasm
CPPC := g++
CC := gcc
WARNINGS := -Wall -Wextra
SIMD := -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4 -mno-avx -mno-avx2
COMMON_FLAGS :=  -O1 -m32 -nostdlib -ffreestanding -mno-red-zone -fno-pie -flto $(SIMD) $(WARNINGS) -T boot2/boot.ld
CPPFLAGS := $(COMMON_FLAGS) -std=c++11 -fno-exceptions -fno-rtti
CFLAGS   := $(COMMON_FLAGS) -std=c11 

CRTI_OBJ = crt/crti.o
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ = crt/crtn.o
CRT_OBJS := $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

ASM_OBJS = build/asm/boot2.o build/asm/interrupt.o build/asm/io.o
OBJS = build/kmain.o build/kerror.o build/paging.o build/interrupt.o build/checkmem.o build/itoa.o build/console.o build/page_map.o build/pic.o build/kstring.o build/kprintf.o build/serial.o build/ps2.o build/keyboard.o build/driver.o build/kalloc.o

.PHONY: all clean depend

all: kernel.img

kernel.img: build/boot0.bin build/boot1.bin build/bootloader.bin 
	@dd if=/dev/zero of=bootloader.img bs=512 count=1024 status=none
	@dd conv=notrunc if=build/boot0.bin of=bootloader.img bs=512 seek=0 status=none
	@dd conv=notrunc if=build/boot1.bin of=bootloader.img bs=512 seek=1 status=none
	@dd conv=notrunc if=build/bootloader.bin of=bootloader.img bs=512 seek=9 status=none

build/boot0.bin:
	@$(NASM) -f bin -Iboot0 boot0/boot0.asm -o $@

build/boot1.bin:
	@$(NASM) -f bin -Iboot1 boot1/boot1.asm -o $@

build/bootloader.bin: $(ASM_OBJS) $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^

build/asm/%.o: boot2/asm/%.asm
	@$(NASM) -f elf32 -Iboot2/asm $< -o $@

build/%.o: boot2/%.cpp
	@$(CPPC) $(CPPFLAGS) -Iboot2 -c $< -o $@

build/%.o: boot2/%.c
	@$(CC) $(CFLAGS) -Iboot2 -c $< -o $@

crt: crt/crt0.o crt/crti.o crt/crtn.o

crt/%.o: crt/%.asm
	@$(NASM) -f elf32 $< -o $@

clean: 
	-@$(RM) build/**.* build/asm/**.* bootloader.img
