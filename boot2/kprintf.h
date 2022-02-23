#pragma once

#include <stdarg.h>
#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC int FASTCALL_MSVC kvprintf(const char* format, va_list vList);
FASTCALL_GCC int FASTCALL_MSVC kvsnprintf(char* buffer, u32 bufferSize, const char* format, va_list vList);

int kprintf(const char* format, ...);
int ksnprintf(char* buffer, u32 bufferSize, const char* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif
