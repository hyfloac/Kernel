#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC u32 FASTCALL_MSVC StrLen(const char* str);

#ifdef __cplusplus
} /* extern "C" */
#endif
