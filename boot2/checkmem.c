#include "checkmem.h"
#include "callspec.h"
#include "console.h"
#include "itoa.h"
#include "kmaths.h"
#include "kprintf.h"
#include "acpi.h"
#include "memcpy.h"
#include "memset.h"

#define MEMORY_TABLE_SIZE_ADDRESS ((uPtr) (0x40000))
#define MEMORY_TABLE_ADDRESS ((uPtr) (MEMORY_TABLE_SIZE_ADDRESS + 8))

static u16 memoryTableSize;
static ACPIMem* memoryTable;

static ACPIMem EbdaRegion;

static void FindEBDA();
static void FindRSDP();

static void MergeAdjacentRegions();
static void CheckOverlappingRegions();
static void SortAndFillHoles();
static void FinalSort();

FASTCALL_GCC static char FASTCALL_MSVC GetDumpACPIChar(ACPIRegionType regionType);
FASTCALL_GCC static ACPIRegionType FASTCALL_MSVC WorseACPIRegionType(ACPIRegionType a, ACPIRegionType b);

#define SwapMemoryRegion(a, b)              \
    { ACPIMem tmp_swap_00 = memoryTable[a]; \
    memoryTable[a] = memoryTable[b];        \
    memoryTable[b] = tmp_swap_00; }

void CheckMemoryLayout()
{
    memoryTableSize = *(u16*) MEMORY_TABLE_SIZE_ADDRESS;
    memoryTable =  (ACPIMem*) MEMORY_TABLE_ADDRESS;

    // Any unknown region types are set to reserved.
    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        if(memoryTable[i].RegionType < ACPI_REGION_MIN || memoryTable[i].RegionType > ACPI_REGION_MAX)
        {
            memoryTable[i].RegionType = ACPI_REGION_UNKNOWN;
        }
    }

    FindEBDA();

    MergeAdjacentRegions();
    CheckOverlappingRegions();
    SortAndFillHoles();
    MergeAdjacentRegions();
    FinalSort();

    *(u16*) MEMORY_TABLE_SIZE_ADDRESS = memoryTableSize;

    FindRSDP();
}

void DumpMemoryLayout()
{
    const ConsoleColor AddressColor = ConColor_LightCyan;
    const ConsoleColor SeperatorColor = ConColor_White;
    const ConsoleColor LengthColor = ConColor_LightBlue;
    const ConsoleColor TypeColor = ConColor_Red;

    {
        const ConCoord cursor = ConGetCursorCoord();
        ConSetCursorCoord(0, cursor.y);
    }

    char addressStore[9];

    for(u32 i = 0; i < memoryTableSize && i < 16; ++i)
    {
        // u32 index = 240 + i * 80;
        ConWriteCharColor(GetDumpACPIChar(memoryTable[i].RegionType), ConColor_Black, TypeColor);
        ConWriteCharColor(' ', ConColor_Black, SeperatorColor);
        ConWriteCharColor('0', ConColor_Black, AddressColor);
        ConWriteCharColor('x', ConColor_Black, AddressColor);

        Xtoap(memoryTable[i].BaseAddressHigh, addressStore);
        ConWriteStringColor(addressStore, ConColor_Black, AddressColor);

        Xtoap(memoryTable[i].BaseAddressLow, addressStore);
        ConWriteStringColor(addressStore, ConColor_Black, AddressColor);

        ConWriteCharColor(':', ConColor_Black, SeperatorColor);
        ConWriteCharColor('0', ConColor_Black, AddressColor);
        ConWriteCharColor('x', ConColor_Black, AddressColor);

        const u64 endAddress = memoryTable[i].BaseAddress + memoryTable[i].RegionLength;

        Xtoap((u32) (endAddress >> 32), addressStore);
        ConWriteStringColor(addressStore, ConColor_Black, AddressColor);

        Xtoap((u32) (endAddress & 0xFFFFFFFF), addressStore);
        ConWriteStringColor(addressStore, ConColor_Black, AddressColor);

        ConWriteCharColor(' ', ConColor_Black, SeperatorColor);
        ConWriteCharColor('[', ConColor_Black, SeperatorColor);
        ConWriteCharColor('0', ConColor_Black, LengthColor);
        ConWriteCharColor('x', ConColor_Black, LengthColor);

        Xtoap(memoryTable[i].RegionLengthHigh, addressStore);
        ConWriteStringColor(addressStore, ConColor_Black, LengthColor);

        Xtoap(memoryTable[i].RegionLengthLow, addressStore);
        ConWriteStringColor(addressStore, ConColor_Black, LengthColor);

        ConWriteCharColor(']', ConColor_Black, SeperatorColor);
        
        {
            const ConCoord cursor = ConGetCursorCoord();
            ConSetCursorCoord(0, cursor.y + 1);
        }
    }
    ConSwapBuffers();
}

static void FindEBDA()
{
    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        // The EBDA can't appear in high memory.
        if(memoryTable[i].BaseAddressHigh != 0 || memoryTable[i].RegionLengthHigh != 0)
        {
            continue;
        }

        // The EBDA is just below 0xA0000
        if(memoryTable[i].BaseAddressLow + memoryTable[i].RegionLengthLow == 0xA0000)
        {
            EbdaRegion = memoryTable[i];
            memoryTable[i].RegionType = ACPI_REGION_EBDA;
            break;
        }
    }
}

static void FindRSDP()
{
    if(EbdaRegion.BaseAddressLow + EbdaRegion.RegionLengthLow != 0xA0000)
    {
        return;
    }

    char searchBuffer[8] = "RSD PTR ";
    const u64 searchQword = *(u64*) searchBuffer;

    kprintf("RSDP Search: 0x%0X%0X\n", searchQword);

    u32 ebdaLength = EbdaRegion.RegionLengthLow;
    u32 ebdaAddress = EbdaRegion.BaseAddressLow;

    ebdaLength = 0xFFFFF - 0x80000 + 1;
    ebdaAddress = 0x80000;

    kprintf("EBDA Address: 0x%0X, Length: 0x%0X\n", ebdaAddress, ebdaLength);

    // Is base page aligned?
    if(ebdaAddress & (16 - 1))
    {
        // Get the offset into the page
        const u32 alignmentDif = 16 - (ebdaAddress - (ebdaAddress & (16 - 1)));
        ebdaAddress += alignmentDif;
        ebdaLength -= alignmentDif;
    }

    kprintf("EBDA Address: 0x%0X, Length: 0x%0X\n", ebdaAddress, ebdaLength);

    ebdaLength /= sizeof(u64);
    const u64* ebda = (u64*) ebdaAddress;
    bool found = false;

    // Search only along 16 byte boundaries.
    for(u32 i = 0; i < ebdaLength; i += 2, ebda += 2)
    {
        if(*ebda == searchQword)
        {
            found = true;
            break;
        }
        else if(i % 4 == 0 && false)
        {
            kprintf("0x%0X%0X 0x%0X%0X 0x%0X%0X 0x%0X%0X\n", *ebda, *(ebda + 1), *(ebda + 2), *(ebda + 3));
            if(i == 64)
            {
                break;
            }
        }
    }

    if(!found)
    {
        kprintf("Failed to find RSDP.\n");
        return;
    }

    const RsdpDescriptor1_0* const rsdp = (const RsdpDescriptor1_0*) ebda;

    char strBuffer[9];
    zeromem_stosb(strBuffer, sizeof(strBuffer));
    memcpy(strBuffer, rsdp->Signature, sizeof(rsdp->Signature));

    kprintf("RSDP: Signature: %s, Checksum: 0x%X, Revision: 0x%X\n", (char*) strBuffer, (u32) rsdp->Checksum, (u32) rsdp->Revision);
    kprintf("RSDP: Revision: 0x%X\n", rsdp->Revision);
    memcpy(strBuffer, rsdp->OEMID, sizeof(rsdp->OEMID));
    strBuffer[sizeof(rsdp->OEMID)] = '\0';

    kprintf("RSDP: OEMID: %s, RSDT Address: 0x%0X\n", (char*) strBuffer, rsdp->RsdtAddress);
}

static void MergeAdjacentRegions()
{
    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        for(u32 j = i + 1; j < memoryTableSize; ++j)
        {
            if(memoryTable[i].BaseAddress + memoryTable[i].RegionLength == memoryTable[j].BaseAddress) // j after i
            {
                if(memoryTable[i].RegionType == memoryTable[j].RegionType)
                {
                    memoryTable[i].RegionLength += memoryTable[j].RegionLength; // Join lengths.
                    
                    --memoryTableSize; // Decrease table size.
                    memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                    --j; // Recheck this index because it contains a different entry
                }
            }
            if(memoryTable[j].BaseAddress + memoryTable[j].RegionLength == memoryTable[i].BaseAddress) // i after j
            {
                if(memoryTable[i].RegionType == memoryTable[j].RegionType)
                {
                    memoryTable[j].RegionLength += memoryTable[i].RegionLength; // Join lengths.
                    
                    --memoryTableSize; // Decrease table size.
                    memoryTable[i] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                    --i; // Recheck this index because it contains a different entry
                    break; // Break the inner loop because we destroyed the previous i.
                }
            }
        }
    }
}

static void CheckOverlappingRegions()
{
    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        for(u32 j = 0; j < memoryTableSize; ++j)
        {
            if(i == j)
            {
                continue;
            }

            if(memoryTable[i].BaseAddress <= memoryTable[j].BaseAddress && memoryTable[i].BaseAddress + memoryTable[i].RegionLength >= memoryTable[j].BaseAddress) // Overlapping Regions
            {
                const u64 iEnd = memoryTable[i].BaseAddress + memoryTable[i].RegionLength;
                const u64 jEnd = memoryTable[j].BaseAddress + memoryTable[j].RegionLength;
                const u64 ijDif = iEnd - memoryTable[j].BaseAddress;

                if(memoryTable[i].BaseAddress == memoryTable[j].BaseAddress) // Begin Region
                {
                    if(memoryTable[i].RegionType == memoryTable[j].RegionType) // Delete region j
                    {
                        --memoryTableSize; // Decrease table size.
                        memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                        --j; // Recheck this index because it contains a different entry
                    }
                    else if(iEnd == jEnd || WorseACPIRegionType(memoryTable[i].RegionType, memoryTable[j].RegionType) == memoryTable[i].RegionType) // Identical size regions, or i contains j, but i is worse
                    {
                        // Take on the worse region size, this only applies if iEnd == jEnd
                        memoryTable[i].RegionType = WorseACPIRegionType(memoryTable[i].RegionType, memoryTable[j].RegionType);

                        // Delete region j
                        --memoryTableSize; // Decrease table size.
                        memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                        --j; // Recheck this index because it contains a different entry
                    }
                    else // Different region sizes, i contains j, but j is worse than i.
                    {
                        memoryTable[i].BaseAddress += ijDif;
                        memoryTable[i].RegionLength -= ijDif;
                    }
                }
                else if(jEnd == iEnd) // End Region
                {
                    if(memoryTable[i].RegionType == memoryTable[j].RegionType) // Delete region j
                    {
                        --memoryTableSize; // Decrease table size.
                        memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                        --j; // Recheck this index because it contains a different entry
                    }
                    else if(WorseACPIRegionType(memoryTable[i].RegionType, memoryTable[j].RegionType) == memoryTable[i].RegionType) // i contains j, but i is worse
                    {
                        // Delete region j
                        --memoryTableSize; // Decrease table size.
                        memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                        --j; // Recheck this index because it contains a different entry
                    }
                    else // i contains j, but j is worse than i.
                    {
                        memoryTable[i].RegionLength -= ijDif;
                    }
                }
                else if(jEnd > iEnd) // Intersecting Region
                {
                    if(WorseACPIRegionType(memoryTable[i].RegionType, memoryTable[j].RegionType) == memoryTable[i].RegionType) // i has the worse region, take possesion of the overlap
                    {
                        memoryTable[j].RegionLength -= memoryTable[j].BaseAddress - iEnd;
                        memoryTable[j].BaseAddress = iEnd;
                    }
                    else  // j has the worse region, take possesion of the overlap
                    {
                        memoryTable[i].RegionLength -= memoryTable[j].BaseAddress - iEnd;
                    }
                }
                else // Middle
                {
                    if(memoryTable[i].RegionType == memoryTable[j].RegionType) // Delete region j
                    {
                        --memoryTableSize; // Decrease table size.
                        memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                        --j; // Recheck this index because it contains a different entry
                    }
                    else if(WorseACPIRegionType(memoryTable[i].RegionType, memoryTable[j].RegionType) == memoryTable[i].RegionType) // i contains j, but i is worse
                    {
                        // Delete region j
                        --memoryTableSize; // Decrease table size.
                        memoryTable[j] = memoryTable[memoryTableSize]; // Copy the last entry to this position.
                        --j; // Recheck this index because it contains a different entry
                    }
                    else // i contains j, but j is worse than i.
                    {
                        memoryTable[memoryTableSize].BaseAddress = jEnd;
                        memoryTable[memoryTableSize].RegionLength = iEnd - jEnd;
                        memoryTable[memoryTableSize].RegionType = memoryTable[i].RegionType;
                        memoryTable[memoryTableSize].ExtendedAttributes = memoryTable[i].ExtendedAttributes;

                        ++memoryTableSize;

                        memoryTable[i].RegionLength = memoryTable[j].BaseAddress - memoryTable[i].BaseAddress;
                    }
                }
            }
        }
    }
}

static void SortAndFillHoles()
{
    u64 targetAddress = 0;

    // Sort and fill holes
    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        if(memoryTable[i].BaseAddress == targetAddress) // Already in the correct position
        {
            targetAddress += memoryTable[i].RegionLength;
            continue;
        }

        u32 closestTarget = i;
        u32 found = 0;

        for(u32 j = i + 1; j < memoryTableSize; ++j)
        {
            if(memoryTable[j].BaseAddress == targetAddress) // Found target
            {
                found = 1;
                SwapMemoryRegion(i, j);
                targetAddress += memoryTable[i].RegionLength;
                break;
            }
            else if(memoryTable[j].BaseAddress < memoryTable[closestTarget].BaseAddress) // The next best target
            {
                closestTarget = j;
            }
        }

        if(!found) // Fill the missing space
        {
            if(memoryTable[closestTarget].RegionType == ACPI_REGION_MISSING) // Holes are treated as Missing, expand this region to fit.
            {
                if(closestTarget != i)
                {
                    SwapMemoryRegion(i, closestTarget);
                }
                memoryTable[i].RegionLength += memoryTable[i].BaseAddress - targetAddress;
                memoryTable[i].BaseAddress = targetAddress;
                targetAddress += memoryTable[i].RegionLength;
            }
            else
            {
                memoryTable[memoryTableSize] = memoryTable[i]; // Move the current region to the end.
                ++memoryTableSize; // Resize the table to fit the new entry.

                const u64 regionLength = memoryTable[closestTarget].BaseAddress - targetAddress;

                memoryTable[i].BaseAddress = targetAddress;
                memoryTable[i].RegionLength = regionLength;
                memoryTable[i].RegionType = ACPI_REGION_MISSING;
                memoryTable[i].ExtendedAttributes = 0;

                targetAddress += memoryTable[i].RegionLength;

                if(closestTarget == i)
                {
                    SwapMemoryRegion(i + 1, memoryTableSize - 1);
                }
                else
                {
                    SwapMemoryRegion(i + 1, closestTarget);
                }

                ++i; // Skip the next entry as it is already correct.
                // if(closestTarget != i)
                // {
                //     SwapMemoryRegion(i, closestTarget);
                // }
                targetAddress += memoryTable[i].RegionLength;
            }
        }
    }
}

static void FinalSort()
{
    u64 targetAddress = 0;

    // Final sort
    for(u32 i = 0; i < memoryTableSize; ++i)
    {
        if(memoryTable[i].BaseAddress == targetAddress) // Already in the correct position
        {
            targetAddress += memoryTable[i].RegionLength;
            continue;
        }

        for(u32 j = i + 1; j < memoryTableSize; ++j)
        {
            if(memoryTable[j].BaseAddress == targetAddress) // Found target
            {
                SwapMemoryRegion(i, j);
                targetAddress += memoryTable[i].RegionLength;
                break;
            }
        }
    }
}

FASTCALL_GCC static char FASTCALL_MSVC GetDumpACPIChar(ACPIRegionType regionType)
{
    switch(regionType)
    {
        case ACPI_REGION_NORMAL:      return 'N';
        case ACPI_REGION_RESERVED:    return 'R';
        case ACPI_REGION_RECLAIMABLE: return 'X';
        case ACPI_REGION_ACPI_NVS:    return 'V';
        case ACPI_REGION_BAD:         return 'B';
        case ACPI_REGION_EBDA:        return 'E';
        case ACPI_REGION_MISSING:     return 'H';
        default:                      return '?';
    }
}

FASTCALL_GCC static ACPIRegionType FASTCALL_MSVC WorseACPIRegionType(ACPIRegionType a, ACPIRegionType b)
{
    if(a == b) // Same Region
    {
        return a;
    }

    if(a == ACPI_REGION_NORMAL) // Normal is the best type, b must be worse
    {
        return b;
    }

    if(b == ACPI_REGION_NORMAL) // Normal is the best type, a must be worse
    {
        return a;
    }

    if(a == ACPI_REGION_RECLAIMABLE) // Reclaimable is the second best type, b must be worse
    {
        return b;
    }

    if(b == ACPI_REGION_RECLAIMABLE) // Reclaimable is the second best type, a must be worse
    {
        return a;
    }

    if(a == ACPI_REGION_BAD || b == ACPI_REGION_BAD) // Bad is the worst type
    {
        return ACPI_REGION_BAD;
    }

    if(a == ACPI_REGION_ACPI_NVS || b == ACPI_REGION_ACPI_NVS) // NVS is the second worst type
    {
        return ACPI_REGION_ACPI_NVS;
    }

    if(a == ACPI_REGION_UNKNOWN || b == ACPI_REGION_UNKNOWN) // We can't know what was intended, assume it's unknown and bad.
    {
        return ACPI_REGION_UNKNOWN;
    }

    return ACPI_REGION_RESERVED; // The only remaining option.
}
