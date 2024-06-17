#!/bin/zsh

qemu-system-i386 -drive file=bootloader.img,format=raw,index=0,media=disk -serial stdio -S -s
# qemu-system-x86_64 -drive file=bootloader.img,format=raw,index=0,media=disk -serial stdio
