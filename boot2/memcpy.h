#pragma once

#include "kstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

inline void memcpy(void* dst, const void* src, const uSys length)
{
    u8* dst8 = (u8*) dst;
    const u8* src8 = (const u8*) src;

    for(uSys i = 0; i < length; ++i)
    {
        *dst8 = *src8;
        ++dst8;
        ++src8;
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif
