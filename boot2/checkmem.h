#pragma once

#include "kstdint.h"
#include "kassert.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
    ACPI_REGION_NORMAL = 1,
    ACPI_REGION_RESERVED = 2,
    ACPI_REGION_RECLAIMABLE = 3,
    ACPI_REGION_ACPI_NVS = 4,
    ACPI_REGION_BAD = 5,
    ACPI_REGION_FORCE_DWORD = 0xFFFFFFFF,
    ACPI_REGION_MIN = ACPI_REGION_NORMAL,
    ACPI_REGION_MAX = ACPI_REGION_BAD
} ACPIRegionType;

typedef struct
{
    union
    {
        struct
        {
            u32 BaseAddressLow;
            u32 BaseAddressHigh;
        };
        u64 BaseAddress;
    };
    union
    {
        struct
        {
            u32 RegionLengthLow;
            u32 RegionLengthHigh;
        };
        u64 RegionLength;
    };
    ACPIRegionType RegionType : 32;
    union
    {
        u32 ExtendedAttributes;
        struct
        {
            u32 AReserved : 30;
            u32 AIgnored : 1;
            u32 ANonVolatile : 1;
        };
    };
} __attribute__((packed)) ACPIMem;

STATIC_ASSERT(sizeof(ACPIMem) == 24, "ACPIMem was not 24 bytes.");

void CheckMemoryLayout();
void DumpMemoryLayout();

#ifdef __cplusplus
} /* extern "C" */
#endif
