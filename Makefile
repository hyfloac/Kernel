NASM := nasm
CPPC := i686-elf-g++
CC := i686-elf-gcc
WARNINGS := -Wall -Wextra -Wno-multichar
DISABLE_SIMD_FLAGS := -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4 -mno-avx -mno-avx2

BOOT2_INCLUDE_FLAGS := -Iboot2
KERNEL_INCLUDE_FLAGS := -Iboot2 -Ikernel/include

COMMON_FLAGS :=  -O1 -m32 -nostdlib -ffreestanding -mno-red-zone $(DISABLE_SIMD_FLAGS) $(WARNINGS)
COMMON_CPPFLAGS := -std=c++2a -fno-exceptions -fno-rtti
COMMON_CFLAGS := -std=c23 -DNO_BOOL -DUSE_SIZED_ENUM

BOOT2_COMMON_FLAGS := -fno-pie -T boot2/boot.ld $(COMMON_FLAGS) $(BOOT2_INCLUDE_FLAGS)
BOOT2_CPPFLAGS := $(BOOT2_COMMON_FLAGS) $(COMMON_CPPFLAGS)
BOOT2_CFLAGS   := $(BOOT2_COMMON_FLAGS) $(COMMON_CFLAGS)

ELF_COMMON_FLAGS := -pie -fPIE -fPIC -T kernel/kernel.ld $(COMMON_FLAGS) $(KERNEL_INCLUDE_FLAGS)
ELF_CPPFLAGS := $(ELF_COMMON_FLAGS) $(COMMON_CPPFLAGS)
ELF_CFLAGS   := $(ELF_COMMON_FLAGS) $(COMMON_CFLAGS)

CRTI_OBJ = crt/crti.o
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ = crt/crtn.o
CRT_OBJS := $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

ASM_OBJS = build/asm/boot2.o build/asm/interrupt.o build/asm/io.o
OBJS0 = build/kmain.o build/kerror.o build/paging.o build/interrupt.o build/checkmem.o build/itoa.o build/console.o build/page_map.o build/pic.o build/kstring.o build/kprintf.o build/serial.o
OBJS1 = build/ps2.o build/keyboard.o build/driver.o build/kalloc.o build/pool_allocator.o build/command_line.o build/mdl.o build/avl_tree.o build/acpi.o build/elf32.o
OBJS = $(OBJS0) $(OBJS1)

KERNEL_ASM_OBJS = build/kernel/asm/start.o
KERNEL_OBJS = build/kernel/kmain.o build/kernel/cpp.o

.PHONY: clean
all: bootloader.iso

bootloader.iso: bootloader.img
	@cp bootloader.img bootloader.iso

bootloader.img: build/mbr.bin build/boot0.bin build/boot1.bin build/bootloader.bin build/kernel.elf
	@dd if=/dev/zero of=bootloader.img bs=512 count=1024 status=none
	@dd conv=notrunc if=build/mbr.bin of=bootloader.img bs=512 seek=0 status=none
	@dd conv=notrunc if=build/boot0.bin of=bootloader.img bs=512 seek=1 status=none
	@dd conv=notrunc if=build/boot1.bin of=bootloader.img bs=512 seek=2 status=none
	@dd conv=notrunc if=build/bootloader.bin of=bootloader.img bs=512 seek=10 status=none
	@dd conv=notrunc if=build/kernel.elf of=bootloader.img bs=512 seek=170 status=none

build/kernel.elf: $(KERNEL_ASM_OBJS) $(KERNEL_OBJS)
	@$(CC) $(ELF_CFLAGS) -o $@ $^

build/mbr.bin: mbr/mbr.asm
	@$(NASM) -f bin -Imbr $^ -o $@

build/boot0.bin: boot0/boot0.asm
	@$(NASM) -f bin -Iboot0 -Imbr $^ -o $@

build/boot1.bin: boot1/boot1.asm
	@$(NASM) -f bin -Iboot1 $^ -o $@

build/bootloader.bin: $(ASM_OBJS) $(OBJS)
	@$(CC) $(BOOT2_CFLAGS) -Xlinker -Map=$@.map -o $@ $^

build/asm/%.o: boot2/asm/%.asm
	@$(NASM) -f elf32 -Iboot2/asm $< -o $@

build/%.o: boot2/%.cpp
	@$(CPPC) $(BOOT2_CPPFLAGS) -T boot2/boot.ld -c $< -o $@

build/%.o: boot2/%.c
	@$(CC) $(BOOT2_CFLAGS) -c $< -o $@

build/kernel/asm/%.o: kernel/asm/%.asm
	@$(NASM) -f elf32 -Ikernel/asm $< -o $@

build/kernel/%.o: kernel/%.cpp
	@$(CPPC) $(ELF_CPPFLAGS) -c $< -o $@

build/kernel/%.o: kernel/%.c
	@$(CC) $(ELF_CFLAGS) -c $< -o $@

crt: crt/crt0.o crt/crti.o crt/crtn.o

crt/%.o: crt/%.asm
	@$(NASM) -f elf32 $< -o $@

clean: 
	-@$(RM) build/**.* build/asm/**.* build/kernel/**.* build/kernel/asm/**.* bootloader.img bootloader.iso
