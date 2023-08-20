#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC u32 FASTCALL_MSVC StrLen(const char* const str);

FASTCALL_GCC i32 FASTCALL_MSVC StrCmpLen(const char* const str0, const uSys str0Length, const char* const str1, const uSys str1Length);

#ifdef __cplusplus
} /* extern "C" */
#endif
