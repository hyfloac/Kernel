#pragma once

#include "kstdint.h"
#include "callspec.h"
#include "kassert.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE (4096)

typedef struct
{
    u32 high;
    u32 low;
} PAEPointer;

static inline u64 PAEPointerToU64(const PAEPointer pointer)
{
    return (((u64) pointer.high) << 32) | (u64) pointer.low;
}

STATIC_ASSERT(sizeof(PAEPointer) == 8, "PAEPointer was not 8 bytes in size.");

extern void enable_pae32();
extern FASTCALL_GCC void FASTCALL_MSVC set_cr3_page_pointer32(void* ptr);
extern void flush_tlb32();

extern void enable_pae64();
extern void set_cr3_page_pointer64(void* ptr);
extern void flush_tlb64();

void SetupPaging32();

PAEPointer GetPhysPage(const void* virtualAddress);
PAEPointer GetPhysAddress(const void* virtualAddress);
void MapPage(u64 physicalAddress, void* virtualAddress, u32 readWrite, u32 userSupervisor, u32 pageLevelWriteThrough, u32 pageLevelCacheDisable);

#ifdef __cplusplus
} /* extern "C" */
#endif
