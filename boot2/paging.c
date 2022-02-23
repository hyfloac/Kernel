#include "paging.h"

#include "kstdint.h"

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
    const u32 PageLevelCacheDisable,
    const u32 physicalAddressLow,
    const u32 physicalAddressHigh
)
{
    PAEPageTableEntry entry;
    entry.Present = present ? 1 : 0;
    entry.ReadWrite = readWrite ? 1 : 0;
    entry.UserSupervsior = userSupervisor ? 1 : 0;
    entry.PageLevelWriteThrough = pageLevelWriteThrough ? 1 : 0;
    entry.PageLevelCacheDisable = PageLevelCacheDisable ? 1 : 0;
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
    (void) CreatePAEPageDirectoryEntry(0, 0, 0, 0, 0, 0, 0);
    (void) CreatePAEPageTableEntry(0, 0, 0, 0, 0, 0, 0);

    ClearPDS();
    InitPDPT();
    InitPagingEntries();
    InitKernelPageEntries();

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

void MapPage(void* const physicalAddress, void* const virtualAddress, const u32 flags)
{
    // Make sure that both addresses are page-aligned.
    const u32 pdindex = (u32) virtualAddress >> 22;
    const u32 ptindex = (u32) virtualAddress >> 12 & 0x03FF;
 
    // u32* const pd = (u32*) 0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.
 
    u32* pt = ((u32*) 0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?
 
    pt[ptindex] = ((u32) physicalAddress) | (flags & 0xFFF) | 0x01; // Present
 
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
    const u32 pdpt1Address = pdsAddress + 4096;
    const u32 pdpt2Address = pdsAddress + 4096 * 2;
    const u32 pdpt3Address = pdsAddress + 4096 * 3;

    PageDirectoryPointerTable[0] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt0Address & 0x000FFFFF, pdpt0Address >> 20);
    PageDirectoryPointerTable[1] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt1Address & 0x000FFFFF, pdpt1Address >> 20);
    PageDirectoryPointerTable[2] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt2Address & 0x000FFFFF, pdpt2Address >> 20);
    PageDirectoryPointerTable[3] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt3Address & 0x000FFFFF, pdpt3Address >> 20);
}

static void InitPagingEntries()
{
    const u32 index = PDPT_COUNT * PDE_COUNT - PDE_COUNT; // The last 4 entries

    u32 pdsAddress = (u32) page_directory_storage;
    pdsAddress /= 4096;

    const u32 pdpt0Address = pdsAddress;
    const u32 pdpt1Address = pdsAddress + 4096;
    const u32 pdpt2Address = pdsAddress + 4096 * 2;
    const u32 pdpt3Address = pdsAddress + 4096 * 3;

    page_directory_storage[index + 0] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt0Address & 0x000FFFFF, pdpt0Address >> 20);
    page_directory_storage[index + 1] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt1Address & 0x000FFFFF, pdpt1Address >> 20);
    page_directory_storage[index + 2] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt2Address & 0x000FFFFF, pdpt2Address >> 20);
    page_directory_storage[index + 3] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, pdpt3Address & 0x000FFFFF, pdpt3Address >> 20);
}

static void InitKernelPageEntries()
{
    const u32 kernelBaseAddress = 0x7C00;
    // const u32 kernelBasePage = kernelBaseAddress / 4096;
    const u32 kernelPageCount = 4;

    for(u32 i = 0; i < kernelPageCount; ++i)
    {
        void* address = (void*) (kernelBaseAddress * 4096 * i);
        MapPage(address, address, 0);
    }
}
