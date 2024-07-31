#pragma once

#include "kstdint.h"
#include "enum.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 Elf32_Addr;
typedef u16 Elf32_Half;
typedef u32 Elf32_Off;
typedef i32 Elf32_Sword;
typedef u32 Elf32_Word;

#define ELF_NIDENT (16)

#pragma pack(push, 1)
typedef struct Elf32EHeader 
{
    u8 e_ident[ELF_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_verson; 
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32EHeader;
#pragma pack(pop)

enum ElfIdentifier 
{
    EI_MAG0       = 0x0,
    EI_MAG1       = 0x1,
    EI_MAG2       = 0x2,
    EI_MAG3       = 0x3,
    EI_CLASS      = 0x4,
    EI_DATA       = 0x5,
    EI_VERSION    = 0x6,
    EI_OSABI      = 0x7,
    EI_ABIVERSION = 0x8,
    EI_PAD0       = 0x9,
    EI_PAD1       = 0xA,
    EI_PAD2       = 0xB,
    EI_PAD3       = 0xC,
    EI_PAD4       = 0xD,
    EI_PAD5       = 0xE,
    EI_PAD6       = 0xF,
};

#define ELF_MAGIC_0 (0x7F)
#define ELF_MAGIC_1 ('E')
#define ELF_MAGIC_2 ('L')
#define ELF_MAGIC_3 ('F')

DECL_ENUM_S(ElfDataClass, u8) 
{
    ELFCLASSNONE = 0,
    ELFCLASS32 = 1,
    ELFCLASS64 = 2
} ElfDataClass;

DECL_ENUM_S(ElfIdentData, u8)
{
    ELFDATANONE = 0,
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2
} ElfIdentData;

DECL_ENUM_S(ElfType, Elf32_Half)
{
    ET_NONE   = 0,
    ET_REL    = 1,
    ET_EXEC   = 2,
    ET_DYN    = 3,
    ET_CORE   = 4,
    ET_LOPROC = 0xFF00,
    ET_HIPROC = 0xFFFF
} ElfType;

DECL_ENUM_S(ElfMachine, Elf32_Half)
{
    EM_NONE   = 0,
    EM_M32    = 1,
    EM_SPARC  = 2,
    EM_386    = 3,
    EM_68K    = 4,
    EM_88K    = 5,
    EM_860    = 7,
    EM_MIPS   = 8
} ElfMachine;

DECL_ENUM_S(ElfVersion, Elf32_Word)
{
    EV_NONE = 0,
    EV_1    = 1,
    EV_CURRENT = EV_1
} ElfVersion;

#pragma pack(push, 1)
typedef struct Elf32SectionHeader
{
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;

} Elf32SectionHeader;
#pragma pack(pop)

DECL_ENUM_S(ElfSectionType, Elf32_Word)
{
    SHT_NULL     = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB   = 2,
    SHT_STRTAB   = 3,
    SHT_RELA     = 4,
    SHT_HASH     = 5,
    SHT_DYNAMIC  = 6,
    SHT_NOTE     = 7,
    SHT_NOBITS   = 8,
    SHT_REL      = 9,
    SHT_SHLIB    = 10,
    SHT_DYNSYM   = 11,
    SHT_LOPROC   = 0x70000000,
    SHT_HIPROC   = 0x7FFFFFFF,
    SHT_LOUSER   = 0x80000000,
    SHT_HIUSER   = 0xFFFFFFFF,
} ElfSectionType;

DECL_ENUM_S(ElfSectionAttributes, Elf32_Word)
{
    SHF_WRITE = 0x1,
    SHF_ALLOC = 0x2,
    SHF_EXECINSTR = 0x4,
    SHF_MASKPROC = 0xF0000000
} ElfSectionAttributes;

static inline Elf32SectionHeader* GetElfSectionHeader(const Elf32EHeader* const header)
{
    return (Elf32SectionHeader*) (((const u8*) header) + header->e_shoff);
}

static inline Elf32SectionHeader* GetElfSection(const Elf32EHeader* const header, const u32 index)
{
    return &GetElfSectionHeader(header)[index];
}


#pragma pack(push, 1)
typedef struct Elf32Symbol
{
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    u8         st_info;
    u8         st_other;
    Elf32_Half st_shndx;
} Elf32Symbol;
#pragma pack(pop)

DECL_ENUM_S(ElfSymbolBinding, u8)
{
    STB_LOCAL  = 0x0,
    STB_GLOBAL = 0x1,
    STB_WEAK   = 0x2,
    STB_LOPROC = 0xD,
    STB_HIPROC = 0xF
} ElfSymbolBinding;

DECL_ENUM_S(ElfSymbolType, u8)
{
    STT_NOTYPE  = 0x0,
    STT_OBJECT  = 0x1,
    STT_FUNC    = 0x2,
    STT_SECTION = 0x3,
    STT_FILE    = 0x4,
    STT_LOPROC  = 0xD,
    STT_HIPROC  = 0xF
} ElfSymbolType;

static inline ElfSymbolBinding ELF32_ST_BIND(const u8 info)
{
    return (ElfSymbolBinding) (info >> 4);
}

static inline ElfSymbolType ELF32_ST_TYPE(const u8 info)
{
    return (ElfSymbolType) (info & 0xF);
}

DECL_ENUM_S(ElfSpecialSectionIndex, Elf32_Half)
{
    SHN_UNDEF     = 0x0000,
    SHN_LORESERVE = 0xFF00,
    SHN_LOPROC    = 0xFF00,
    SHN_HIPROC    = 0xFF1F,
    SHN_ABS       = 0xFFF1,
    SHN_COMMON    = 0xFFF2,
    SHN_HIRESERVE = 0xFFFF
} ElfSpecialSectionIndex;

DECL_ENUM_S(ElfSymbolTableNumber, int)
{
    STN_UNDEF = 0
} ElfSymbolTableNumber;


#pragma pack(push, 1)
typedef struct Elf32Relocation
{
    Elf32_Addr r_offset;
    Elf32_Word r_info;
} Elf32Relocation;

typedef struct Elf32RelocationAddend
{
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
    Elf32_Sword r_addend;
} Elf32RelocationAddend;
#pragma pack(pop)

/**
 * A   - This means the addend used to compute the value of the relocatable field.
 * B   -   This means the base address at which a shared object has been loaded into memory during
 *       execution. Generally, a shared object file is built with a 0 base virtual address, but the 
 *       execution address will be different.
 * G   -   This means the offset into the global offset table at which the address of the relocation entry’s
 *       symbol will reside during execution. See ‘‘Global Offset Table’’ in Part 2 for more information.
 * GOT -   This means the address of the global offset table. See ‘‘Global Offset Table’’ in Part 2 for more
 *       information
 * L   -   This means the place (section offset or address) of the procedure linkage table entry for a 
 *       symbol. A procedure linkage table entry redirects a function call to the proper destination. The
 *       link editor builds the initial procedure linkage table, and the dynamic linker modifies the
 *       entries during execution. See ‘‘Procedure Linkage Table’’ in Part 2 for more information.
 * P   -   This means the place (section offset or address) of the storage unit being relocated (computed
 *       using r_offset).
 * S   - This means the value of the symbol whose index resides in the relocation entry.
 */
DECL_ENUM_S(ElfRelocationType, u8)
{
    R_386_NONE      =  0, // none, none
    R_386_32        =  1, // word32, S + A
    R_386_PC32      =  2, // word32, S + A - P
    R_386_GOT32     =  3, // word32, G + A - P
    R_386_PLT32     =  4, // word32, L + A - P
    R_386_COPY      =  5, // none, none
    R_386_GLOB_DAT  =  6, // word32, S
    R_386_JMP_SLOT  =  7, // word32, S
    R_386_RELATIVE  =  8, // word32, B + A
    R_386_GOTOFF    =  9, // word32, S + A - GOT
    R_386_GOTPC     = 10, // word32, GOT + A - P
} ElfRelocationType;

static inline Elf32_Word ELF32_R_SYM(const Elf32_Word info)
{
    return (info >> 8);
}

static inline ElfRelocationType ELF32_R_TYPE(const Elf32_Word info)
{
    return (ElfRelocationType) ((u8) (info & 0xFF));
}

void* Elf32LoadFile(void* file);

#ifdef __cplusplus
} /* extern "C" */
#endif
