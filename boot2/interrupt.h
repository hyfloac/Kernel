#pragma once

#include "kstdint.h"
#include "callspec.h"
#include "kassert.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u16 AddressLow;
    u16 Segment;
    u8 Reserved;
    u8 Attributes;
    u16 AddressHigh;
} __attribute__((packed)) IDTEntry32;

// typedef struct
// {
//     u16 AddressLow;
//     u16 Segment;
//     u8 IST;
//     u8 Attributes;
//     u16 AddressMid;
//     u32 AddressHigh;
//     u32 Reserved;
// } __attribute__((packed)) IDTEntry64;

typedef struct 
{
    u16 Limit;
    u32 Base;
} __attribute__((packed)) IDTR32;

STATIC_ASSERT(sizeof(IDTEntry32) == 8, "IDTEntry32 was not 8 bytes in size.");
STATIC_ASSERT(sizeof(IDTR32) == 6, "IDTEntry32 was not 6 bytes in size.");

typedef struct
{
    u32 cr4;
    u32 cr3;
    u32 cr2;
    u32 cr0;

    u32 edi;
    u32 esi;
    u32 edx;
    u32 ecx;
    u32 ebx;
    u32 eax;

    u32 ebp;
    u32 Vector;
    u32 ErrorCode;
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 dss;
} IsrExceptionFrame;


// typedef struct 
// {
//     u16 Limit;
//     u64 Base;
// } __attribute__((packed)) IDTR64;

static inline IDTEntry32 CreateIDTEntry32(const u32 address, const u16 segment, const u8 attributes)
{
    IDTEntry32 entry;
    entry.AddressLow = address & 0xFFFF;
    entry.Segment = segment;
    entry.Reserved = 0;
    entry.Attributes = attributes;
    entry.AddressHigh = address >> 16;
    return entry;
}

// static inline IDTEntry64 CreateIDTEntry64(const u64 address, const u16 segment, const u8 ist, const u8 attributes)
// {
//     IDTEntry64 entry;
//     entry.AddressLow = address & 0xFFFF;
//     entry.Segment = segment;
//     entry.IST = ist;
//     entry.Attributes = attributes;
//     entry.AddressMid = (address >> 16) & 0xFFFF;
//     entry.AddressHigh = address >> 32;
//     entry.Reserved = 0;
//     return entry;
// }

static inline IDTR32 CreateIDTR32(const u16 limit, const u32 base)
{
    IDTR32 idtr;
    idtr.Limit = limit;
    idtr.Base = base;
    return idtr;
}

// static inline IDTR64 CreateIDTR64(const u16 limit, const u64 base)
// {
//     IDTR64 idtr;
//     idtr.Limit = limit;
//     idtr.Base = base;
//     return idtr;
// }

void InitIDT32();
void LoadIDT32();

FASTCALL_GCC extern void FASTCALL_MSVC load_idt32(IDTR32* idt);
// extern void load_idt64(IDTR64* idt);

#ifdef __cplusplus
} /* extern "C" */
#endif
