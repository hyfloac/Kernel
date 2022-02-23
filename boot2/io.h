#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC extern void FASTCALL_MSVC out8(u16 port, u8  val);
FASTCALL_GCC extern void FASTCALL_MSVC out16(u16 port, u16 val);
FASTCALL_GCC extern void FASTCALL_MSVC out32(u16 port, u32 val);
FASTCALL_GCC extern u8  FASTCALL_MSVC in8(u16 port);
FASTCALL_GCC extern u16 FASTCALL_MSVC in16(u16 port);
FASTCALL_GCC extern u32 FASTCALL_MSVC in32(u16 port);
extern void io_wait();

#ifdef __cplusplus
}
#endif
