#pragma once

#include "kstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u32 high;
    u32 low;
} PAEPointer;

extern void enable_pae();
extern void set_cr3_page_pointer(void* ptr);
extern void flush_tlb();

void setupPaging();

PAEPointer getPhysAddress(const void* virtualAddress);
void mapPage(void* physicalAddress, void* virtualAddress, u32 flags);

#ifdef __cplusplus
} /* extern "C" */
#endif
