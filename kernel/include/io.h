#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void out8(u16 port, u8  val);
extern void out16(u16 port, u16 val);
extern void out32(u16 port, u32 val);
extern u8  in8(u16 port);
extern u16 in16(u16 port);
extern u32 in32(u16 port);
extern void io_wait();

#ifdef __cplusplus
}
#endif
