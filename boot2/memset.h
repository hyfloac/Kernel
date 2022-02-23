#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC extern void FASTCALL_MSVC memset_stosb(void* memory, u8 value, u32 count);
FASTCALL_GCC extern void FASTCALL_MSVC zeromem_stosb(void* memory, u32 count);

#ifdef __cplusplus
} /* extern "C" */
#endif
