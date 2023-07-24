#pragma once

#include "callspec.h"
#include "kstdint.h"
#include "kerror.h"

#ifdef __cplusplus
extern "C" {
#endif

KError_t InitPageMap();

FASTCALL_GCC u64 FASTCALL_MSVC GetPhysPages(u32* pPageCount);
FASTCALL_GCC u64 FASTCALL_MSVC GetPhysPages32Bit(u32* pPageCount);

#ifdef __cplusplus
} /* extern "C" */
#endif
