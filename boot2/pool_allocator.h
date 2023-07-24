#pragma once

#include "enum.h"
#include "kstdint.h"
#include "kerror.h"

DECL_ENUM(PoolFlag)
{
    PoolFlagPaged = 0,
    PoolFlagNonPaged = 1 << 0,

    PoolFlagExecutable = 1 << 1,
    PoolFlagZeroMemory = 1 << 2,

    PoolFlagPagedExec = PoolFlagPaged | PoolFlagExecutable,
    PoolFlagNonPagedExec = PoolFlagNonPaged | PoolFlagExecutable,
};

void* AllocatePoolWithTag(PoolFlag Flags, uSys SizeInBytes, u32 Tag);

typedef struct PoolInfo
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
    u32 PoolTag;
} PoolInfo;

KError_t EnumPools(uSys* PoolCount, PoolInfo* PoolInfos);
