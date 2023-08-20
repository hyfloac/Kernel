#include "paging.h"

#include "kstdint.h"
#include "page_map.h"
#include "kprintf.h"

#ifndef LOG_MAP_PAGE
  #define LOG_MAP_PAGE (0)
#endif

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
        u32 Dirty : 1;
        u32 PAT : 1;
        u32 Global : 1;
        u32 Ignored : 3;
        u32 PhysicalAddressLow : 20;
        u32 PhysicalAddressHigh : 31;
        u32 Reserved : 1;
    };
    u64 _packed;
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

static __attribute__((aligned(32))) PageDirectoryPointerTableEntry PageDirectoryPointerTable[4];
static PAEPageDirectoryEntry* page_directory_storage; // 16 KiB, 4 directories, 512 entries each
static PAEPageDirectoryEntry* PageDirectoriesVirtual;
/// Virtually maps to all of our page tables.
static PAEPageTableEntry* PageTableMapPhysical; // 16 KiB, 4 Page Tables, 512 entries each
static PAEPageTableEntry* PageTableMapVirtual;
static PAEPageTableEntry* AllPageTableEntries;

#define PDPT_COUNT (4)
#define PDE_COUNT (512)
#define PTE_COUNT PDE_COUNT
#define PT_COUNT (1)
#define PD_BASE_ADDRESS (0xFFFFC000)

static void ClearPDS();
static void InitPDPT();
static void InitKernelPageEntries();

void SetupPaging32()
{
    u32 pageCount = 4;
    page_directory_storage = (PAEPageDirectoryEntry*) (u32) GetPhysPages32Bit(&pageCount);
    pageCount = 4;
    PageTableMapPhysical = (PAEPageTableEntry*) (u32) GetPhysPages32Bit(&pageCount);

    (void) CreatePAEPageDirectoryEntry(0, 0, 0, 0, 0, 0, 0);
    (void) CreatePAEPageTableEntry(0, 0, 0, 0, 0, 0, 0);

    ClearPDS();
    InitPDPT();
    InitKernelPageEntries();

#if 0
    PAEPageDirectoryEntry* firstPageDirectory = (PAEPageDirectoryEntry*) (PageDirectoryPointerTable[0].PhysicalAddressLow << 12);
    PAEPageTableEntry* firstPageTable = (PAEPageTableEntry*) (firstPageDirectory[0].PhysicalAddressLow << 12);

    u64 firstPageAddress = ((((u64) firstPageTable[0].PhysicalAddressHigh) << 32) | firstPageTable[0].PhysicalAddressLow) << 12;
    u64 secondPageAddress = ((((u64) firstPageTable[1].PhysicalAddressHigh) << 32) | firstPageTable[1].PhysicalAddressLow) << 12;

    kprintf("First  page address (should be 0x0000_0000_0000_0000) 0x%p%p\n", (u32) (firstPageAddress >> 32), (u32) firstPageAddress);
    kprintf("Second page address (should be 0x0000_0000_0001_0000) 0x%p%p\n", (u32) (secondPageAddress >> 32), (u32) secondPageAddress);

    u64 pdpt0Address = ((((u64) PageDirectoryPointerTable[0].PhysicalAddressHigh) << 32) | PageDirectoryPointerTable[0].PhysicalAddressLow) << 12;

    kprintf("0x%p%p\n", (u32) (pdpt0Address >> 32), (u32) pdpt0Address);
    kprintf("0x%p\n", PageDirectoryPointerTable);
#endif

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
    

    // const u32 pdindex = (u32) virtualAddress >> 22
    const u32 ptIndex = (u32) virtualAddress >> 12;
 
    // u32* const pd = (u32*) 0xFFFFF000;
    // Here you need to check whether the PD entry is present.
 
    // u32* const pt = ((u32*) 0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.

    PAEPageTableEntry* pte = &AllPageTableEntries[ptIndex];
 
    u64 physAddress = (((u64) pte->PhysicalAddressHigh) << 20) | pte->PhysicalAddressLow;
    physAddress <<= 12;
    physAddress |= (((u32) virtualAddress) & 0x0FFF);
    PAEPointer ret = { (u32) (physAddress >> 32), (u32) (physAddress & 0xFFFFFFFF) };
    // PAEPointer ret = { pte->PhysicalAddressHigh, pte->PhysicalAddressLow };
    return ret;
}

void MapPage(u64 const physicalAddress, void* const virtualAddress, const u32 readWrite, const u32 userSupervisor, const u32 pageLevelWriteThrough, const u32 pageLevelCacheDisable)
{
    /*
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
    */

    const u32 pdIndex = (u32) virtualAddress >> 21;
    const u32 ptMapIndex = (u32) virtualAddress >> 12;

    #if LOG_MAP_PAGE
    kprintf("Mapping 0x%0X%0X to 0x%p\n", (u32) (physicalAddress >> 32), (u32) physicalAddress, virtualAddress);
    kprintf("Index in the sequential Page Directory that may need to be mapped: 0x%X\n", pdIndex);
    kprintf("Index in the sequential Page Table Map that needs to be mapped: 0x%X\n", ptMapIndex);
    #endif

    // Do we need to create a new page table?
    if(PageDirectoriesVirtual[pdIndex]._packed == 0)
    {
        #if LOG_MAP_PAGE
        kprintf("Need to map a page into the page directory.\n");
        #endif

        u32 pageCount = 1;
        u64 newPTPageNumber = GetPhysPages(&pageCount) >> 12;

        #if LOG_MAP_PAGE
        kprintf("Mapping 0x%0X%0X to index 0x%X in the Page Directory and Page Map.\n", (u32) (newPTPageNumber >> 20), (u32) (newPTPageNumber << 12), pdIndex);
        kprintf("Address 0x%p in the Directories\n", &PageDirectoriesVirtual[pdIndex]);
        kprintf("Address 0x%p in the Page Table Map\n", &PageTableMapVirtual[pdIndex]);
        #endif

        PageDirectoriesVirtual[pdIndex] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, newPTPageNumber & 0x000FFFFF, newPTPageNumber >> 20);

        #if LOG_MAP_PAGE
        kprintf("Page Directory Mapped.\n");
        #endif

        PageTableMapVirtual[pdIndex] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, newPTPageNumber & 0x000FFFFF, newPTPageNumber >> 20);

        #if LOG_MAP_PAGE
        kprintf("Page Table Mapping Mapped.\n");
        #endif
    }

    // PAEPageTableEntry* pt = ((PAEPageTableEntry*) 0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?
 
    // pt[ptindex] = ((u32) physicalAddress) | (flags & 0xFFF) | 0x01; // Present

    const u64 phsyicalAddressPageNumber  = physicalAddress >> 12;

    #if LOG_MAP_PAGE
    kprintf("Setting the mapping at index 0x%X 0x%p\n", ptMapIndex, &AllPageTableEntries[ptMapIndex]);
    #endif

    AllPageTableEntries[ptMapIndex] = CreatePAEPageTableEntry(1, readWrite, userSupervisor, pageLevelWriteThrough, pageLevelCacheDisable, (u32) (phsyicalAddressPageNumber & 0x000FFFFF), (u32) (phsyicalAddressPageNumber >> 20));
 
    #if LOG_MAP_PAGE
    kprintf("Page has been mapped.\n");
    #endif

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

    for(u32 i = 0; i < count; ++i)
    {
        PageTableMapPhysical[i]._packed = 0;
    }
}

/// @brief Setup the PDPT with our 4 PDs
static void InitPDPT()
{
    // The page number of our 4 PDEs
    const u32 pdsAddress = ((u32) page_directory_storage) >> 12;

    const u32 pdpt0Address = pdsAddress;
    const u32 pdpt1Address = pdsAddress + 1;
    const u32 pdpt2Address = pdsAddress + 2;
    const u32 pdpt3Address = pdsAddress + 3;

    PageDirectoryPointerTable[0] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt0Address & 0x000FFFFF, pdpt0Address >> 20);
    PageDirectoryPointerTable[1] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt1Address & 0x000FFFFF, pdpt1Address >> 20);
    PageDirectoryPointerTable[2] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt2Address & 0x000FFFFF, pdpt2Address >> 20);
    PageDirectoryPointerTable[3] = CreatePageDirectoryPointerTableEntry(1, 0, 0, pdpt3Address & 0x000FFFFF, pdpt3Address >> 20);
}

static void InitKernelPageEntries()
{
    // Get two pages, these will be the first 2 Page Tables.
    u32 pageCount = 2;
    PAEPageTableEntry* firstTable = (PAEPageTableEntry*) (u32) GetPhysPages32Bit(&pageCount);

    PAEPageTableEntry* secondTable = firstTable + (PTE_COUNT);

    // If we received only 1 page, get a second.
    if(pageCount == 1)
    {
        secondTable = (PAEPageTableEntry*) (u32) GetPhysPages32Bit(&pageCount);
    }

    // Identity map the entirety of the first Page Table.
    for(u32 i = 0; i < PTE_COUNT; ++i)
    {
        firstTable[i] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, i, 0);
    }

    // Identity map everything but the last 4 entries of the second Page Table.
    for(u32 i = 0; i < PTE_COUNT - 4; ++i)
    {
        secondTable[i] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, i + PTE_COUNT, 0);
    }

    // The page number of our 4 PDs
    const u32 pdsAddress = ((u32) page_directory_storage) >> 12;
    
    const u32 pd0Address = pdsAddress;
    const u32 pd1Address = pdsAddress + 1;
    const u32 pd2Address = pdsAddress + 2;
    const u32 pd3Address = pdsAddress + 3;

    // Set the next last entries to our Page Directories
    secondTable[(PTE_COUNT) - 4] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, pd0Address & 0x000FFFFF, pd0Address >> 20);
    secondTable[(PTE_COUNT) - 3] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, pd1Address & 0x000FFFFF, pd1Address >> 20);
    secondTable[(PTE_COUNT) - 2] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, pd2Address & 0x000FFFFF, pd2Address >> 20);
    secondTable[(PTE_COUNT) - 1] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, pd3Address & 0x000FFFFF, pd3Address >> 20);

    const u32 firstPTAddress = ((u32) firstTable) >> 12;
    const u32 secondPTAddress = ((u32) secondTable) >> 12;
    const u32 mapPTAddress = ((u32) PageTableMapPhysical) >> 12;
    
    const u32 mapPT0Address = mapPTAddress;
    const u32 mapPT1Address = mapPTAddress + 1;
    const u32 mapPT2Address = mapPTAddress + 2;
    const u32 mapPT3Address = mapPTAddress + 3;

    // Map the first 2 entries of the Page Table Map to our 2 Page Tables.
    PageTableMapPhysical[0] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, firstPTAddress, 0);
    PageTableMapPhysical[1] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, secondPTAddress, 0);
    // Map the last 4 Page Table Map entries to themself.
    PageTableMapPhysical[(PDPT_COUNT) * (PDE_COUNT) - 4] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, mapPT0Address & 0x000FFFFF, mapPT0Address >> 20);
    PageTableMapPhysical[(PDPT_COUNT) * (PDE_COUNT) - 3] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, mapPT1Address & 0x000FFFFF, mapPT1Address >> 20);
    PageTableMapPhysical[(PDPT_COUNT) * (PDE_COUNT) - 2] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, mapPT2Address & 0x000FFFFF, mapPT2Address >> 20);
    PageTableMapPhysical[(PDPT_COUNT) * (PDE_COUNT) - 1] = CreatePAEPageTableEntry(1, 1, 0, 0, 0, mapPT3Address & 0x000FFFFF, mapPT3Address >> 20);

    // Map the first 2 Page Directory Entries to our 2 Page Tables
    page_directory_storage[0] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, firstPTAddress, 0);
    page_directory_storage[1] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, secondPTAddress, 0);

    // Map the Page Table Map to the last 4 pages.
    page_directory_storage[(PDPT_COUNT) * (PDE_COUNT) - 4] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, mapPT0Address & 0x000FFFFF, mapPT0Address >> 20);
    page_directory_storage[(PDPT_COUNT) * (PDE_COUNT) - 3] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, mapPT1Address & 0x000FFFFF, mapPT1Address >> 20);
    page_directory_storage[(PDPT_COUNT) * (PDE_COUNT) - 2] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, mapPT2Address & 0x000FFFFF, mapPT2Address >> 20);
    page_directory_storage[(PDPT_COUNT) * (PDE_COUNT) - 1] = CreatePAEPageDirectoryEntry(1, 1, 0, 0, 0, mapPT3Address & 0x000FFFFF, mapPT3Address >> 20);

    PageDirectoriesVirtual = (PAEPageDirectoryEntry*) 0x003FC000;
    PageTableMapVirtual = (PAEPageTableEntry*) 0xFFFFC000;
    AllPageTableEntries = (PAEPageTableEntry*) 0xFF800000;

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
