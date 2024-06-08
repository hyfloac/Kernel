#include "mdl.h"
#include "pool_allocator.h"
#include "paging.h"

#define POOL_TAG_MDL ('LDMk')

MDL* MmAllocMdl(void* const virtualAddress, const u64 length)
{
    u64 blockCount = 1;

    const u8* virtualAddress8 = (u8*) virtualAddress;

    u64 lastPage = PAEPointerToU64(GetPhysPage(virtualAddress8));
    virtualAddress8 += PAGE_SIZE;

    i64 i;
    for(i = (i64) length - PAGE_SIZE; i >= PAGE_SIZE; i -= PAGE_SIZE, virtualAddress8 += PAGE_SIZE)
    {
        const u64 currPage = PAEPointerToU64(GetPhysPage(virtualAddress8));

        if(currPage != lastPage + PAGE_SIZE)
        {
            ++blockCount;
        }
        lastPage = currPage;
    }

    if(i > 0)
    {
        virtualAddress8 += i;

        const u64 currPage = PAEPointerToU64(GetPhysPage(virtualAddress8));

        if(currPage != lastPage + PAGE_SIZE)
        {
            ++blockCount;
        }
    }

    MDL* mdl = AllocatePoolWithTag(PoolFlagNonPaged | PoolFlagZeroMemory, sizeof(MDL) + sizeof(MDLBlock) * blockCount, POOL_TAG_MDL);

    if(!mdl)
    {
        return nullptr;
    }

    mdl->pNext = nullptr;
    mdl->BlockCount = blockCount;
    mdl->VirtualAddress = virtualAddress;

    MDLBlock* currentBlock = (MDLBlock*) (mdl + 1);
    currentBlock->PageCount = 1;

    virtualAddress8 = (u8*) virtualAddress;

    lastPage = PAEPointerToU64(GetPhysPage(virtualAddress8));
    currentBlock->StartPage = lastPage;
    virtualAddress8 += PAGE_SIZE;

    for(i = (i64) length - PAGE_SIZE; i >= PAGE_SIZE; i -= PAGE_SIZE, virtualAddress8 += PAGE_SIZE)
    {
        const u64 currPage = PAEPointerToU64(GetPhysPage(virtualAddress8));

        if(currPage != lastPage + PAGE_SIZE)
        {
            ++currentBlock;
            currentBlock->StartPage = currPage;
            currentBlock->PageCount = 1;
        }
        else
        {
            ++currentBlock->PageCount;
        }
        lastPage = currPage;
    }

    if(i > 0)
    {
        virtualAddress8 += i;

        const u64 currPage = PAEPointerToU64(GetPhysPage(virtualAddress8));

        if(currPage != lastPage + PAGE_SIZE)
        {
            ++currentBlock;
            currentBlock->StartPage = currPage;
            currentBlock->PageCount = 1;
        }
        else
        {
            ++currentBlock->PageCount;
        }
    }

    return mdl;
}

void MmFreeMdl(MDL* mdl)
{
    FreePoolWithTag(mdl, POOL_TAG_MDL);
}
