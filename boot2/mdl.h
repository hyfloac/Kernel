#pragma once

#include "kstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MDLBlock
{
    u64 StartPage;
    u64 PageCount;
} MDLBlock;

typedef struct MDL
{
    struct MDL* pNext;
    u64 BlockCount;
    void* VirtualAddress;
} MDL;

MDL* MmAllocMdl(void* virtualAddress, u64 length);
void MmFreeMdl(MDL* mdl);

#ifdef __cplusplus
} /* extern "C" */
#endif
