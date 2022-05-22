#include "checkmem.h"
#include "page_map.h"
#include "callspec.h"
#include "bitset.h"
#include "memset.h"
#include "kprintf.h"
#include "console.h"

/**
 * The number of blocks in the memory table.
 * This does not include the filtered valid blocks.
 */
static u16 memoryTableSize;
/**
 * The table of memory blocks.
 */
static ACPIMem* memoryTable;

/**
 * The number of valid blocks that can be used.
 * These blocks occur are found after memoryTable[memoryTableSize].
 */
static u16 validBlockCount;

typedef struct PageLinkedList
{
    u64 Page;
    struct PageLinkedList* Next;
} PageLinkedList;

static PageLinkedList* P1Head;
static PageLinkedList* P2Head;
static PageLinkedList* P4Head;
static PageLinkedList* P8Head;
static PageLinkedList* P16Head;
static PageLinkedList* P32Head;
static PageLinkedList* P64Head;
static PageLinkedList* P128Head;
static PageLinkedList* P256Head;

static PageLinkedList** PHeads[] = {
    &P1Head,
    &P2Head,
    &P4Head,
    &P8Head,
    &P16Head,
    &P32Head,
    &P64Head,
    &P128Head,
    &P256Head,
};

FASTCALL_GCC static i16 FASTCALL_MSVC LocatePageIndex(const u64 address, u32* const bitIndex);
FASTCALL_GCC static void FASTCALL_MSVC SetUsedPages(const u64 address, const u32 pageCount);
FASTCALL_GCC static void FASTCALL_MSVC RedistributePageHeads(PageLinkedList* head, u32 pageCount);
static void InitPLLArena();
static PageLinkedList* AllocPLL();
FASTCALL_GCC static void FASTCALL_MSVC FreePLL(PageLinkedList* pll);

KError_t InitPageMap()
{
    memoryTableSize = *(u16*) 0x40000;
    memoryTable =  (ACPIMem*) 0x40008;
    validBlockCount = 0;

    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        if(memoryTable[i].RegionType == ACPI_REGION_NORMAL)
        {
            // Ignore the base pages for safety. This might not be necessary.
            if(memoryTable[i].BaseAddress < 0x9FC00)
            {
                continue;
            }

            const u32 index = memoryTableSize + validBlockCount;

            memoryTable[index] = memoryTable[i];

            const u32 base = memoryTable[index].BaseAddress;

            // Is base page aligned?
            if(base & 0x1000)
            {
                // Get the offset into the page
                const u32 alignmentDif = 0x1000 - (base - (base & 0x1000));
                memoryTable[index].BaseAddress = base + alignmentDif;
                memoryTable[index].RegionLength -= alignmentDif;
            }

            memoryTable[index].RegionLength &= ~0x1000; // Align length.

            const u32 pageCount = memoryTable[index].RegionLength / 0x1000;
            const u32 bitsetWordCount = BitSet_Words(pageCount);
            const u32 bitsetByteCount = bitsetWordCount * sizeof(u32);
            
            if(memoryTable[index].BaseAddressHigh == 0)
            {
                // Zero out the bitset of used pages.
                zeromem_stosb((void*) memoryTable[index].BaseAddressLow, bitsetByteCount);
                const u32 minInfoPages = bitsetByteCount / 0x1000;
                const u32 slackBytes = bitsetByteCount % 0x1000;
                const u32 infoPages = minInfoPages + (slackBytes != 0 ? 1 : 0);
                for(u32 i = 0; i < infoPages; ++i)
                {
                    // Set all pages used by the bitset to used.
                    BitSet_SetBit((void*) memoryTable[index].BaseAddressLow, i);
                }
            }

            ++validBlockCount;
        }
    }

    if(!validBlockCount)
    {
        return KE_PAGING_NO_VALID_PAGES;
    }

    if(memoryTable[memoryTableSize].BaseAddressHigh != 0)
    {
        return KE_PAGING_NO_32_BIT_PAGES;
    }

    InitPLLArena();

#define FILL_BLOCK(_BLOCK_SIZE, _INDEX) \
    if(j >= _BLOCK_SIZE) {                      \
        PageLinkedList* pll = AllocPLL();       \
        if(!pll) {                              \
            return KE_PAGING_NO_MORE_PLL;       \
        }                                       \
        pll->Page = currentAddress;             \
        pll->Next = *(PHeads[_INDEX]);          \
        *(PHeads[_INDEX]) = pll;                \
        currentAddress += 0x1000 * _BLOCK_SIZE; \
        j -= _BLOCK_SIZE;                       \
    }

    for(u32 i = 0; i < validBlockCount; ++i)
    {
        kprintf("Initializing Block %u/%u\n", i + 1, validBlockCount);
        ACPIMem* block = &memoryTable[memoryTableSize + i];
        const u64 pageCount = block->RegionLength / 0x1000;

        u64 currentAddress = block->BaseAddress;
        kprintf("  Page Count: %u", pageCount);
        for(u64 j = pageCount; j > 0;)
        {
            FILL_BLOCK(256, 8);
            FILL_BLOCK(128, 7);
            FILL_BLOCK(64, 6);
            FILL_BLOCK(32, 5);
            FILL_BLOCK(16, 4);
            FILL_BLOCK(8, 3);
            FILL_BLOCK(4, 2);
            FILL_BLOCK(2, 1);
            FILL_BLOCK(1, 0);
        }
        kprintf("\nFinsihed allocating blocks.\n");
    }
#undef FILL_BLOCK

    return KE_OK;
}

FASTCALL_GCC u64 FASTCALL_MSVC GetPhysPages(u32* const pPageCount)
{
    if(!pPageCount)
    {
        return 0;
    }

    const u32 pageCount = *pPageCount;

    if(!pageCount)
    {
        return 0;
    }

    if(pageCount == 1)
    {
        if(P1Head)
        {
            PageLinkedList* const head = P1Head;
            const u64 ret = head->Page;
            P1Head = P1Head->Next;
            FreePLL(head);
            SetUsedPages(ret, 1);
            return ret;
        }
    }

    if(pageCount <= 2)
    {
        if(P2Head)
        {
            PageLinkedList* const head = P2Head;
            const u64 ret = head->Page;
            P2Head = P2Head->Next;

            if(pageCount == 1)
            {
                head->Page += 4096;
                head->Next = P1Head;
                P1Head = head;
            }
            else
            {
                FreePLL(head);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    if(pageCount <= 4)
    {
        if(P4Head)
        {
            PageLinkedList* const head = P4Head;
            const u64 ret = head->Page;
            P4Head = P4Head->Next;

            if(pageCount == 4)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 4 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    if(pageCount <= 8)
    {
        if(P8Head)
        {
            PageLinkedList* const head = P8Head;
            const u64 ret = head->Page;
            P8Head = P8Head->Next;

            if(pageCount == 8)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 8 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }

    if(pageCount <= 16)
    {
        if(P16Head)
        {
            PageLinkedList* const head = P16Head;
            const u64 ret = head->Page;
            P16Head = P16Head->Next;

            if(pageCount == 16)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 16 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    if(pageCount <= 32)
    {
        if(P32Head)
        {
            PageLinkedList* const head = P32Head;
            const u64 ret = head->Page;
            P32Head = P32Head->Next;

            if(pageCount == 32)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 32 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    if(pageCount <= 64)
    {
        if(P64Head)
        {
            PageLinkedList* const head = P64Head;
            const u64 ret = head->Page;
            P64Head = P64Head->Next;

            if(pageCount == 64)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 64 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    if(pageCount <= 128)
    {
        if(P128Head)
        {
            PageLinkedList* const head = P128Head;
            const u64 ret = head->Page;
            P128Head = P128Head->Next;

            if(pageCount == 128)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 128 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    if(pageCount <= 256)
    {
        if(P256Head)
        {
            PageLinkedList* const head = P256Head;
            const u64 ret = head->Page;
            P256Head = P256Head->Next;

            if(pageCount == 256)
            {
                FreePLL(head);
            }
            else
            {
                const u32 dif = 256 - pageCount;
                head->Page += 4096 * pageCount;
                RedistributePageHeads(head, dif);
            }
            SetUsedPages(ret, pageCount);
            return ret;
        }
    }
    
    {
        i32 index = 8;

        while(index >= 0)
        {
            if(*(PHeads[index]))
            {
                PageLinkedList* head = *(PHeads[index]);
                const u64 ret = head->Page;
                *(PHeads[index]) = head->Next;
                FreePLL(head);
                *pPageCount = 1 << index;
                SetUsedPages(ret, *pPageCount);
                return ret;
            }
            --index;
        }

        return 0;
    }
}

FASTCALL_GCC u64 FASTCALL_MSVC GetPhysPages32Bit(u32* const pPageCount)
{
    #define IS_VALID_PAGE(ADDR) ((ADDR) <= (0xFFFFFFFFull - (pageCount * 4096)))

    if(!pPageCount)
    {
        return 0;
    }

    const u32 pageCount = *pPageCount;

    if(!pageCount)
    {
        return 0;
    }

    if(pageCount == 1)
    {
        if(P1Head)
        {
            PageLinkedList* prev = P1Head;

            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P1Head = P1Head->Next;
                FreePLL(prev);
                SetUsedPages(ret, 1);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = curr->Page;
                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;
                    FreePLL(curr);
                    SetUsedPages(ret, 1);
                    return ret;
                }
                prev = curr;
            }
        }
    }

    if(pageCount <= 2)
    {
        if(P2Head)
        {
            PageLinkedList* prev = P2Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P2Head = P2Head->Next;

                if(pageCount == 1)
                {
                    prev->Page += 4096;
                    prev->Next = P1Head;
                    P1Head = prev;
                }
                else
                {
                    FreePLL(prev);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = curr->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 1)
                    {
                        curr->Page += 4096;
                        curr->Next = P1Head;
                        P1Head = prev;
                    }
                    else
                    {
                        FreePLL(curr);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }

        }
    }
    
    if(pageCount <= 4)
    {
        if(P4Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P4Head = P4Head->Next;

                if(pageCount == 4)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 4 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 4)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 4 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }
    
    if(pageCount <= 8)
    {
        if(P8Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P8Head = P8Head->Next;

                if(pageCount == 8)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 8 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 8)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 8 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }

    if(pageCount <= 16)
    {
        if(P16Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P16Head = P16Head->Next;

                if(pageCount == 16)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 16 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 16)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 16 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }
    
    if(pageCount <= 32)
    {
        if(P32Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P32Head = P32Head->Next;

                if(pageCount == 32)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 32 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 32)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 32 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }
    
    if(pageCount <= 64)
    {
        if(P64Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P64Head = P64Head->Next;

                if(pageCount == 64)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 64 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 64)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 64 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }
    
    if(pageCount <= 128)
    {
        if(P128Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P128Head = P128Head->Next;

                if(pageCount == 128)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 128 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 128)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 128 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }
    
    if(pageCount <= 256)
    {
        if(P256Head)
        {
            PageLinkedList* prev = P4Head;
            if(IS_VALID_PAGE(prev->Page))
            {
                const u64 ret = prev->Page;
                P256Head = P256Head->Next;

                if(pageCount == 256)
                {
                    FreePLL(prev);
                }
                else
                {
                    const u32 dif = 256 - pageCount;
                    prev->Page += 4096 * pageCount;
                    RedistributePageHeads(prev, dif);
                }
                SetUsedPages(ret, pageCount);
                return ret;
            }

            while(prev->Next)
            {
                PageLinkedList* const curr = prev->Next;
                const u64 ret = prev->Page;

                if(IS_VALID_PAGE(ret))
                {
                    prev->Next = curr->Next;

                    if(pageCount == 256)
                    {
                        FreePLL(curr);
                    }
                    else
                    {
                        const u32 dif = 256 - pageCount;
                        curr->Page += 4096 * pageCount;
                        RedistributePageHeads(curr, dif);
                    }
                    SetUsedPages(ret, pageCount);
                    return ret;
                }

                prev = curr;
            }
        }
    }
    
    // Find the largest set of pages available.
    {
        i32 index = 8;

        while(index >= 0)
        {
            if(*(PHeads[index]))
            {
                PageLinkedList* prev = *(PHeads[index]);
                if(IS_VALID_PAGE(prev->Page))
                {
                    const u64 ret = prev->Page;
                    *(PHeads[index]) = prev->Next;

                    FreePLL(prev);
                    *pPageCount = 1 << index;
                    SetUsedPages(ret, *pPageCount);
                    return ret;
                }

                while(prev->Next)
                {
                    PageLinkedList* const curr = prev->Next;
                    const u64 ret = prev->Page;

                    if(IS_VALID_PAGE(ret))
                    {
                        prev->Next = curr->Next;

                        FreePLL(curr);
                        *pPageCount = 1 << index;
                        SetUsedPages(ret, *pPageCount);
                        return ret;
                    }

                    prev = curr;
                }
            }
            --index;
        }

        return 0;
    }

    #undef IS_VALID_PAGE
}

/**
 * Finds the bit for where a page is.
 * 
 * @return
 *      The index within the valid memory blocks.
 */
FASTCALL_GCC static i16 FASTCALL_MSVC LocatePageIndex(const u64 address, u32* const bitIndex)
{
    {
        if(address < memoryTable[memoryTableSize].BaseAddress)
        {
            return -1;
        }
        const u32 lastIndex = memoryTableSize + validBlockCount - 1;

        if(address > memoryTable[lastIndex].BaseAddress + memoryTable[lastIndex].RegionLength)
        {
            return -1;
        }
    }

    u32 currentIndex = validBlockCount / 2;
    u32 minIndex = 0;
    u32 maxIndex = validBlockCount - 1;

    while(1)
    {
        if(address < memoryTable[memoryTableSize + currentIndex].BaseAddress)
        {
            maxIndex = currentIndex;
            currentIndex = (currentIndex + minIndex) / 2;
        }
        else
        {
            const u64 realIndex = memoryTableSize + currentIndex;
            const u64 maxAddress = memoryTable[realIndex].BaseAddress + memoryTable[realIndex].RegionLength;
            if(address > maxAddress)
            {
                minIndex = currentIndex;
                currentIndex = (currentIndex + maxIndex) / 2;
            }
            else
            {
                if(bitIndex)
                {
                    const u64 relAddress = address - memoryTable[realIndex].BaseAddress;
                    *bitIndex = (u32) (relAddress / 4096);
                }
                
                return (i16) currentIndex;
            }
        }
    }

    return -1; // Not Possible.
}

FASTCALL_GCC static void FASTCALL_MSVC SetUsedPages(const u64 address, const u32 pageCount)
{
    u32 bitIndex = 0xFFFFFFFF;
    const i16 blockIndex = LocatePageIndex(address, &bitIndex);

    if(blockIndex > -1 && bitIndex != 0xFFFFFFFF)
    {
        for(u32 i = 0; i < pageCount; ++i)
        {
            BitSet_SetBit((void*) memoryTable[blockIndex].BaseAddressLow, bitIndex + i);
        }
    }
}

FASTCALL_GCC static void FASTCALL_MSVC RedistributePageHeads(PageLinkedList* head, u32 pageCount)
{
    u32 headIndex = 0;

    while(pageCount)
    {
        if(pageCount & 0x1)
        {
            if(pageCount & ~0x1)
            {
                PageLinkedList* pll = AllocPLL();
                pll->Page = head->Page;
                pll->Next = *(PHeads[headIndex]);
                *(PHeads[headIndex]) = pll;

                head->Page += 4096 * (headIndex + 1);
            }
            else
            {
                head->Next = *(PHeads[headIndex]);
                *(PHeads[headIndex]) = head;
            }
        }
        pageCount >>= 1;
        ++headIndex;
    }
}

#define PLL_BEGIN_POINTER ((void*)(0x50000))
#define PLL_COUNT (0x4000)

static PageLinkedList* FreeHead;

#include "console.h"
#include "itoa.h"

static void InitPLLArena()
{
    FreeHead = PLL_BEGIN_POINTER; // Init PLL head to safely usable memory

    PageLinkedList* current = FreeHead;

    for(u32 i = 0; i < PLL_COUNT; ++i) 
    {
        current->Next = &FreeHead[i]; // Treat the freehead as an array of PLL's. Then set the next PLL as that address.
        current = current->Next;
    }
    current->Next = NULL;
}

static PageLinkedList* AllocPLL()
{
    if(FreeHead)
    {
        PageLinkedList* head = FreeHead;
        FreeHead = FreeHead->Next;
        head->Next = NULL;
        return head;
    }
    else
    {
        return NULL;
    }
}

FASTCALL_GCC static void FASTCALL_MSVC FreePLL(PageLinkedList* pll)
{
    if(!pll)
    {
        return;
    }

    pll->Page = 0;
    pll->Next = FreeHead;
    FreeHead = pll;
}
