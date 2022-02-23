#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC void FASTCALL_MSVC btoa(u32 x, char str[33]);
FASTCALL_GCC void FASTCALL_MSVC btoap(u32 x, char str[33]);
FASTCALL_GCC void FASTCALL_MSVC xtoa(u32 x, char str[9]);
FASTCALL_GCC void FASTCALL_MSVC Xtoa(u32 x, char str[9]);
FASTCALL_GCC void FASTCALL_MSVC xtoap(u32 x, char str[9]);
FASTCALL_GCC void FASTCALL_MSVC Xtoap(u32 x, char str[9]);
FASTCALL_GCC void FASTCALL_MSVC utoa(u32 x, char str[11]);
FASTCALL_GCC void FASTCALL_MSVC itoa(i32 x, char str[12]);

#ifdef __cplusplus
} /* extern "C" */
#endif
