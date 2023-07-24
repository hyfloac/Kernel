#!/bin/zsh

qemu-system-i386 -drive file=bootloader.img,format=raw,index=0,media=disk -serial stdio
