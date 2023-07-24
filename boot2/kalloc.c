#include "kalloc.h"
#include "page_map.h"
#include "paging.h"

#include "kprintf.h"

#define BASE_ALLOC_ADDRESS (0xC0000000)

static u32 currentAllocAddress = BASE_ALLOC_ADDRESS;

// A lazy as fuck allocator that just gives you pages.
void* kalloc(const uSys size)
{
    u32 pageCount = (u32) (size / 4096);

    if(size % 4096 != 0)
    {
        ++pageCount;
    }

    u32 remainingPages = pageCount;

    void* retAddress = (void*) currentAllocAddress;

    kprintf("kalloc Address: 0x%p\n", kalloc);

    do
    {
        u32 physPageCount = remainingPages;

        const u64 physicalAddress = GetPhysPages32Bit(&physPageCount);

        MapPage(physicalAddress, (void*) currentAllocAddress, 1, 0, 0, 0);
        kprintf("Physical Address: 0x%x\n", (u32) physicalAddress);

        remainingPages -= physPageCount;

        currentAllocAddress += physPageCount * 4096;
    } while(remainingPages);
    
    return retAddress;
}
