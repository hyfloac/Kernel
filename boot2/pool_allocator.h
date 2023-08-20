#pragma once

#include "enum.h"
#include "kstdint.h"
#include "kerror.h"

#ifdef __cplusplus
extern "C" {
#endif

DECL_ENUM(PoolFlag)
{
    PoolFlagPaged = 0,
    PoolFlagNonPaged = 1 << 0,

    PoolFlagNonExecutable = 0,
    PoolFlagExecutable = 1 << 1,

    PoolFlagUninitialized = 0,
    PoolFlagZeroMemory = 1 << 2,
    PoolFlagCacheAligned = 1 << 3,

    PoolFlagPagedExec = PoolFlagPaged | PoolFlagExecutable,
    PoolFlagNonPagedExec = PoolFlagNonPaged | PoolFlagExecutable,
};

void InitPoolAllocator(void);

void* AllocatePoolWithTag(const PoolFlag Flags, const uSys SizeInBytes, const u32 Tag);

void FreePoolWithTag(void* const Allocation, const u32 Tag);

typedef struct TaggedAllocationsInfo
{
    u32 CurrentAllocationCount;
    u32 TotalAllocations;
    u32 TotalFrees;
    union
    {
        struct
        {
            u32 TotalAllocationSizeLow;
            u32 TotalAllocationSizeHigh;
        };
        uSys TotalAllocationSize;
    };
    u32 Tag;
} TaggedAllocationsInfo;

KError_t EnumTaggedAllocationInfos(uSys* const TagCount, TaggedAllocationsInfo* const TaggedAllocationInfos);

#ifdef __cplusplus
} /* extern "C" */
#endif
