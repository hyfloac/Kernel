#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC void FASTCALL_MSVC PICRemap(u8 offset1, u8 offset2);
void PICDisable();
FASTCALL_GCC void FASTCALL_MSVC PICSendEOI(u8 irq);

FASTCALL_GCC void FASTCALL_MSVC PICSetMask(u8 irqLine);
FASTCALL_GCC void FASTCALL_MSVC PICClearMask(u8 irqLine);

u16 PICGetIRR();
u16 PICGetISR();

#ifdef __cplusplus
} /* extern "C" */
#endif
