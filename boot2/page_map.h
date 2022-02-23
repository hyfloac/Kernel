#pragma once

#pragma once

#include "callspec.h"
#include "kstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    PageMap_Success = 0,
    PageMap_NoValidPages = 1,
    PageMap_No32BitPages = 2,
    PageMap_NoMorePLL = 3
} PageMapError;

int InitPageMap();

FASTCALL_GCC u64 FASTCALL_MSVC GetPhysPages(u32* pPageCount);

#ifdef __cplusplus
} /* extern "C" */
#endif
