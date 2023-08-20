#include "kalloc.h"
#include "page_map.h"
#include "paging.h"

#include "kprintf.h"

#define BASE_ALLOC_ADDRESS (0xC0000000)

static u32 currentAllocAddress;

void InitKAlloc(void)
{
    currentAllocAddress = BASE_ALLOC_ADDRESS;

    u32 physicalPageCount = 1;
    const u64 physicalAddress = GetPhysPages(&physicalPageCount);

    MapPage(physicalAddress, (void*) currentAllocAddress, 1, 0, 0, 0);
}

static u32 AlignTo(const u32 val, const u32 alignment)
{
    if(alignment == 1 || val % alignment == 0)
    { return val; }
    return (val + alignment) & ~(alignment - 1);
}

// A lazy as fuck allocator that just gives you pages.
void* kalloc(const uSys size)
{
    const u32 endAddress = AlignTo(currentAllocAddress + size, 8);

    const u32 currentPage = (currentAllocAddress >> 12);
    const u32 lastPage = (endAddress >> 12);

    void* const retAddress = (void*) currentAllocAddress;

    u32 virtualAddress = (currentPage + 1) << 12;

    for(u32 remainingPages = lastPage - currentPage; remainingPages > 0;)
    {
        u32 physPageCount = remainingPages;

        const u64 physicalAddress = GetPhysPages(&physPageCount);

        for(u32 i = 0; i < physPageCount; ++i)
        {
            MapPage(physicalAddress + (i << 12), (void*) virtualAddress, 1, 0, 0, 0);

            virtualAddress += 4096;
        }

        remainingPages -= physPageCount;
    };
    
    currentAllocAddress = endAddress;

    return retAddress;
}

void kfree(void* data)
{
    (void) data;
    // Nope.
}
