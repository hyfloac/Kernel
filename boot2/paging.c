#include "paging.h"

#include "kstdint.h"
#include "page_map.h"
#include "kprintf.h"

typedef struct
{
    u32 Present : 1;
    u32 ReadWrite : 1;
    u32 UserSupervsior : 1;
    u32 PageLevelWriteThrough : 1;
    u32 PageLevelCacheDisable : 1;
    u32 Accessed : 1;
    u32 Dirty : 1;
    u32 PAT : 1;
    u32 Global : 1;
    u32 Ignored : 3;
    u32 PhysicalAddressLow : 20;
    u32 PhysicalAddressHigh : 31;
    u32 Reserved : 1;
} __attribute__((packed)) PAEPageTableEntry;

typedef union
{
    struct
    {
        u32 Present : 1;
        u32 ReadWrite : 1;
        u32 UserSupervsior : 1;
        u32 PageLevelWriteThrough : 1;
        u32 PageLevelCacheDisable : 1;
        u32 Accessed : 1;
        u32 Ignored0 : 1;
        u32 PS : 1;
        u32 Ignored1 : 4;
        u32 PhysicalAddressLow : 20;
        u32 PhysicalAddressHigh : 31;
        u32 Reserved : 1;
    };
    u64 _packed;
} __attribute__((packed)) PAEPageDirectoryEntry;

typedef struct
{
    u32 Present : 1;
    u32 Reserved0 : 2;
    u32 PageLevelWriteThrough : 1;
    u32 PageLevelCacheDisable : 1;
    u32 Reserved1 : 4;
    u32 Ignored : 3;
    u32 PhysicalAddressLow : 20;
    u32 PhysicalAddressHigh;
} __attribute__((packed)) PageDirectoryPointerTableEntry;

static PAEPageTableEntry CreatePAEPageTableEntry(
    const u32 present, 
    const u32 readWrite, 
    const u32 userSupervisor,
    const u32 pageLevelWriteThrough, 
    const u32 pageLevelCacheDisable,
    const u32 physicalAddressLow,
    const u32 physicalAddressHigh
)
{
    PAEPageTableEntry entry;
    entry.Present = present ? 1 : 0;
    entry.ReadWrite = readWrite ? 1 : 0;
    entry.UserSupervsior = userSupervisor ? 1 : 0;
    entry.PageLevelWriteThrough = pageLevelWriteThrough ? 1 : 0;
    entry.PageLevelCacheDisable = pageLevelCacheDisable ? 1 : 0;
    entry.Accessed = 0;
    entry.Dirty = 0;
    entry.PAT = 0;
    entry.Ignored = 0;
    entry.PhysicalAddressLow = physicalAddressLow;
    entry.PhysicalAddressHigh = physicalAddressHigh;
    entry.Reserved = 0;

    return entry;
}

static PAEPageDirectoryEntry CreatePAEPageDirectoryEntry(
    const u32 present, 
    const u32 readWrite, 
    const u32 userSupervisor,
    const u32 pageLevelWriteThrough, 
    const u32 PageLevelCacheDisable,
    const u32 physicalAddressLow,
    const u32 physicalAddressHigh
)
{
    PAEPageDirectoryEntry entry;
    entry.Present = present ? 1 : 0;
    entry.ReadWrite = readWrite ? 1 : 0;
    entry.UserSupervsior = userSupervisor ? 1 : 0;
    entry.PageLevelWriteThrough = pageLevelWriteThrough ? 1 : 0;
    entry.PageLevelCacheDisable = PageLevelCacheDisable ? 1 : 0;
    entry.Accessed = 0;
    entry.Ignored0 = 0;
    entry.PS = 0;
    entry.Ignored1 = 0;
    entry.PhysicalAddressLow = physicalAddressLow;
    entry.PhysicalAddressHigh = physicalAddressHigh;
    entry.Reserved = 0;

    return entry;
}

static PageDirectoryPointerTableEntry CreatePageDirectoryPointerTableEntry(
    const u32 present, 
    const u32 pageLevelWriteThrough, 
    const u32 PageLevelCacheDisable,
    const u32 physicalAddressLow,
    const u32 physicalAddressHigh
)
{
    PageDirectoryPointerTableEntry entry;
    entry.Present = present ? 1 : 0;
    entry.Reserved0 = 0;
    entry.PageLevelWriteThrough = pageLevelWriteThrough ? 1 : 0;
    entry.PageLevelCacheDisable = PageLevelCacheDisable ? 1 : 0;
    entry.Reserved1 = 0;
    entry.Ignored = 0;
    entry.PhysicalAddressLow = physicalAddressLow;
    entry.PhysicalAddressHigh = physicalAddressHigh;

    return entry;
}

static PageDirectoryPointerTableEntry PageDirectoryPointerTable[4];
extern PAEPageDirectoryEntry* page_directory_storage; // 16KiB, 4 tables, 512 entries each

#define PDPT_COUNT (4)
#define PDE_COUNT (512)
#define PD_BASE_ADDRESS (0xFFFFC000)

static void ClearPDS();
static void InitPagingEntries();
static void InitPDPT();
static void InitKernelPageEntries();

void SetupPaging32()
{
    u32 pageCount = 4;
    u64 physicalPages = GetPhysPages(&pageCount);
    page_directory_storage = (void*) (u32) physicalPages;

    (void) CreatePAEPageDirectoryEntry(0, 0, 0, 0, 0, 0, 0);
    (void) CreatePAEPageTableEntry(0, 0, 0, 0, 0, 0, 0);

    ClearPDS();
    InitPDPT();
    InitPagingEntries();
    InitKernelPageEntries();


    PAEPageDirectoryEntry* pd = (PAEPageDirectoryEntry*) (PageDirectoryPointerTable[0].PhysicalAddressLow << 12);
    PAEPageTableEntry* pt = (PAEPageTableEntry*) (pd[0].PhysicalAddressLow << 12);

    u64 pageAddress = ((((u64) pt[1].PhysicalAddressHigh) << 32) | pt[1].PhysicalAddressLow) << 12;

    kprintf("0x%p%p\n", (u32) (pageAddress >> 32), (u32) pageAddress);

    u64 pdpt0Address = ((((u64) PageDirectoryPointerTable[0].PhysicalAddressHigh) << 32) | PageDirectoryPointerTable[0].PhysicalAddressLow) << 12;

    kprintf("0x%p%p\n", (u32) (pdpt0Address >> 32), (u32) pdpt0Address);

    enable_pae32();
    set_cr3_page_pointer32(PageDirectoryPointerTable);
}

PAEPointer GetPhysAddress(const void* const virtualAddress)
{  
    // const u32 pdptIndex =  ((u32) virtualAddress) >> 2;
    // const u32   pdIndex = (((u32) virtualAddress) >> 21) & 0x01FF;
    // const u32   ptIndex = (((u32) virtualAddress) >> 12) & 0x01FF;

    // PageDirectoryPointerTableEntry pdpt = PageDirectoryPointerTable[pdptIndex];

    // if(!pdpt.Present)
    // {
    //     PAEPointer ret = { 0, 0 };
    //     return ret;
    // }

    // PAEPageDirectoryEntry* const pd = (PAEPageDirectoryEntry*) (PD_BASE_ADDRESS + 4096 * pdptIndex);
    // PAEPageDirectoryEntry* const pde = pd + pdIndex;

    // if(!(pd->Present))
    // {
    //     PAEPointer ret = { 0, 0 };
    //     return ret;
    // }
    

    const u32 pdindex = (u32) virtualAddress >> 22;
    const u32 ptindex = (u32) virtualAddress >> 12 & 0x03FF;
 
    // u32* const pd = (u32*) 0xFFFFF000;
    // Here you need to check whether the PD entry is present.
 
    u32* const pt = ((u32*) 0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
 
    PAEPointer ret = { 0, ((pt[ptindex] & ~0xFFF) + ((u32) virtualAddress & 0xFFF)) };
    return ret;
}

void MapPage(void* const physicalAddress, void* const virtualAddress, const u32 readWrite, const u32 userSupervisor, const u32 pageLevelWriteThrough, const u32 pageLevelCacheDisable)
{
    // Make sure that both addresses are page-aligned.
    const u32 pdptIndex = (u32) virtualAddress >> 30;
    const u32 pdIndex = (u32) virtualAddress >> 22 & 0x02FF;
    const u32 ptIndex = (u32) virtualAddress >> 12 & 0x02FF;
 
    // u32* const pd = (u32*) 0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.
 


    PAEPageDirectoryEntry* pd = (PAEPageDirectoryEntry*) (PageDirectoryPointerTable[pdptIndex].PhysicalAddressLow << 12);

    PAEPageTableEntry* pt = (PAEPageTableEntry*) (pd[pdIndex].PhysicalAddressLow << 12);

    // PAEPageTableEntry* pt = ((PAEPageTableEntry*) 0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?
 
    // pt[ptindex] = ((u32) physicalAddress) | (flags & 0xFFF) | 0x01; // Present
    pt[ptIndex] = CreatePAEPageTableEntry(1, readWrite, userSupervisor, pageLevelWriteThrough, pageLevelCacheDisable, (u32) physicalAddress >> 12, 0);
 
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
    flush_tlb32();
}

static void ClearPDS()
{
    const u32 count = PDPT_COUNT * PDE_COUNT;

    for(u32 i = 0; i < count; ++i)
    {
        page_directory_storage[i]._packed = 0;
    }
}

static void InitPDPT()
{
    u32 pdsAddress = (u32) page_directory_storage;
    pdsAddress /= 4096;

    const u32 pdpt0Address = pdsAddress;
    const u32 pdpt1Address = pdsAddress + 1;
    const u32 pdpt2Address = pdsAddress + 2;
    const u32 pdpt3Address = pdsAddress + 3;

    PageDirectoryPointerTable[0] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt0Address & 0x000FFFFF, pdpt0Address >> 20);
    PageDirectoryPointerTable[1] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt1Address & 0x000FFFFF, pdpt1Address >> 20);
    PageDirectoryPointerTable[2] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt2Address & 0x000FFFFF, pdpt2Address >> 20);
    PageDirectoryPointerTable[3] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt3Address & 0x000FFFFF, pdpt3Address >> 20);
}

static void InitPagingEntries()
{
    const u32 index = PDPT_COUNT * PDE_COUNT - 4; // The last 4 entries

    u32 pdsAddress = (u32) page_directory_storage;
    pdsAddress /= 4096;

    const u32 pdpt0Address = pdsAddress;
    const u32 pdpt1Address = pdsAddress + 1;
    const u32 pdpt2Address = pdsAddress + 2;
    const u32 pdpt3Address = pdsAddress + 3;

    page_directory_storage[index + 0] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt0Address & 0x000FFFFF, pdpt0Address >> 20);
    page_directory_storage[index + 1] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt1Address & 0x000FFFFF, pdpt1Address >> 20);
    page_directory_storage[index + 2] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt2Address & 0x000FFFFF, pdpt2Address >> 20);
    page_directory_storage[index + 3] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt3Address & 0x000FFFFF, pdpt3Address >> 20);
}

static void InitKernelPageEntries()
{
    u32 pageCount = 1;
    PAEPageTableEntry* firstTable = (PAEPageTableEntry*) (u32) GetPhysPages(&pageCount);

    u32 ptAddress = (u32) firstTable;
    ptAddress /= 4096;

    for(u32 i = 0; i < PDE_COUNT - 1; ++i)
    {
        firstTable[i] = CreatePAEPageTableEntry(1, 1, 1, 0, 0, i, 0);
    }

    firstTable[PDE_COUNT - 1] = CreatePAEPageTableEntry(1, 1, 1, 0, 0, ptAddress, 0);
    page_directory_storage[0] = CreatePAEPageDirectoryEntry(1, 1, 1, 0, 0, ptAddress, 0);

    // const u32 kernelBaseAddress = 0x7C00;
    // // const u32 kernelBasePage = kernelBaseAddress / 4096;
    // const u32 kernelPageCount = 11;

    // for(u32 i = 0; i < kernelPageCount; ++i)
    // {
    //     void* address = (void*) (kernelBaseAddress * 4096 * i);
    //     MapPage(address, address, 1, 1, 0, 0);
    // }

    // void* kernelBackBufferAddress = (void*) 0x0004F000;
    // MapPage(kernelBackBufferAddress, kernelBackBufferAddress, 1, 1, 0, 0);
    // void* kernelFrontBufferAddress = (void*) 0x000B8000;
    // MapPage(kernelFrontBufferAddress, kernelFrontBufferAddress, 1, 1, 0, 0);

    // const u32 pllBaseAddress = 0x00050000;
    // const u32 pllPageCount = 47;

    // for(u32 i = 0; i < pllPageCount; ++i)
    // {
    //     void* address = (void*) (pllBaseAddress * 4096 * i);
    //     MapPage(address, address, 1, 1, 0, 0);
    // }
}
