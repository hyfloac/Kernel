#include "elf32.h"
#include "kprintf.h"
#include "pool_allocator.h"
#include "memcpy.h"

#define TAG_ELF ('KFLE')

static bool ElfCheckHeader(const Elf32EHeader* const header)
{
    if(!header)
    {
        return false;
    }

    if(header->e_ident[EI_MAG0] != ELF_MAGIC_0)
    {
        kprintf("Elf Header EI_MAG0 incorrect: 0x%X != 0x%X\n", header->e_ident[EI_MAG0], ELF_MAGIC_0);
        return false;
    }

    if(header->e_ident[EI_MAG1] != ELF_MAGIC_1)
    {
        kprintf("Elf Header EI_MAG1 incorrect: 0x%X != 0x%X\n", header->e_ident[EI_MAG1], ELF_MAGIC_1);
        return false;
    }

    if(header->e_ident[EI_MAG2] != ELF_MAGIC_2)
    {
        kprintf("Elf Header EI_MAG2 incorrect: 0x%X != 0x%X\n", header->e_ident[EI_MAG2], ELF_MAGIC_2);
        return false;
    }

    if(header->e_ident[EI_MAG3] != ELF_MAGIC_3)
    {
        kprintf("Elf Header EI_MAG3 incorrect: 0x%X != 0x%X\n", header->e_ident[EI_MAG3], ELF_MAGIC_3);
        return false;
    }

    return true;
}

static bool ElfIsSupported(const Elf32EHeader* const header)
{
    if(!header)
    {
        return false;
    }

    if(header->e_ident[EI_CLASS] != ELFCLASS32)
    {
        kprintf("Elf Header EI_CLASS is unsupported: 0x%X != 0x%X\n", header->e_ident[EI_CLASS], ELFCLASS32);
        return false;
    }

    if(header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        kprintf("Elf Header EI_DATA is unsupported: 0x%X != 0x%X\n", header->e_ident[EI_DATA], ELFDATA2LSB);
        return false;
    }

    if(header->e_ident[EI_VERSION] != EV_CURRENT)
    {
        kprintf("Elf Header EI_VERSION is unsupported: 0x%X != 0x%X\n", header->e_ident[EI_VERSION], EV_CURRENT);
        return false;
    }

    if(header->e_machine != EM_386)
    {
        kprintf("Elf Header e_machine is unsupported: 0x%X != 0x%X\n", header->e_machine, EM_386);
        return false;
    }

    if(header->e_type != ET_REL && header->e_type != ET_EXEC)
    {
        kprintf("Elf Header e_type is unsupported: 0x%X != 0x%X,0x%X\n", header->e_type, ET_REL, ET_EXEC);
        return false;
    }

    return true;
}

void* ElfLookupSymbol(const char* name)
{
    (void) name;
    return nullptr;
}

static uPtr GetElfSymbolValue(const Elf32EHeader* const header, const u32 table, const u32 index)
{
    if(table == SHN_UNDEF || index == STN_UNDEF)
    {
        return 0;
    }

    const Elf32SectionHeader* const symbolTable = GetElfSection(header, table);

    const u32 symbolCount = symbolTable->sh_size / symbolTable->sh_entsize;

    if(index >= symbolCount)
    {
        kprintf("Symbol index %u is out of range (%u).\n", index, symbolCount);
        return (uPtr) -1;
    }

    const u8* const symbolRawPtr = ((const u8*) header) + symbolTable->sh_offset;
    const Elf32Symbol* const symbol = &((const Elf32Symbol*) symbolRawPtr)[index];

    if(symbol->st_shndx == SHN_UNDEF)
    {
        // External symbol, lookup value
        const Elf32SectionHeader* const stringTable = GetElfSection(header, symbolTable->sh_link);
        const char* const name = ((const char*) header) + stringTable->sh_offset + symbol->st_name;

        void* const target = ElfLookupSymbol(name);

        if(!target)
        {
            if((ELF32_ST_BIND(symbol->st_info) & STB_WEAK) == STB_WEAK)
            {
                // Weak symbol is initialized as 0.
                return 0;
            }
            else
            {
                kprintf("Undefined external symbol \"%s\"\n", name);
                return (uPtr) -1;
            }
        }
        else
        {
            return (uPtr) target;
        }
    }
    else if(symbol->st_shndx == SHN_ABS)
    {
        // Absolute symbol
        return symbol->st_value;
    }
    else
    {
        // Internally defined symbol
        const Elf32SectionHeader* const target = GetElfSection(header, symbol->st_shndx);
        return ((uPtr) header) + target->sh_offset + symbol->st_value;
    }
}

static int ElfLoadStage1(const Elf32EHeader* const header)
{
    Elf32SectionHeader* const sectionHeader = GetElfSectionHeader(header);

    for(u32 i = 0; i < header->e_shnum; ++i)
    {
        Elf32SectionHeader* const section = &sectionHeader[i];

        if(section->sh_type != SHT_NOBITS)
        {
            continue;
        }

        if(section->sh_size == 0)
        {
            continue;
        }

        if((section->sh_flags & SHF_ALLOC) != SHF_ALLOC)
        {
            continue;
        }

        void* const mem = AllocatePoolWithTag(PoolFlagNonPaged | PoolFlagNonExecutable | PoolFlagZeroMemory, section->sh_size, TAG_ELF);

        section->sh_offset = (Elf32_Off) mem - (Elf32_Off) header;
        kprintf("Allocated memory for section %d.\n", i);
    }

    return 0;
}

#define DO_386_32(S, A)	((S) + (A))
#define DO_386_PC32(S, A, P)	((S) + (A) - (P))

static int ElfPerformRelocation(const Elf32EHeader* const header, Elf32Relocation* const relocation, Elf32SectionHeader* const section)
{
    Elf32SectionHeader* const target = GetElfSection(header, section->sh_info);

    const int addr = (int) header + target->sh_offset;
    int* const ref = (int*) (addr + relocation->r_offset);

    int symval = 0;
    if(ELF32_R_SYM(relocation->r_info) != SHN_UNDEF)
    {
        symval = GetElfSymbolValue(header, section->sh_link, ELF32_R_SYM(relocation->r_info));
        if(symval < 0)
        {
            return symval;
        }
    }

    switch(ELF32_R_TYPE(relocation->r_info))
    {
        case R_386_NONE: break;
        case R_386_32:
            *ref = DO_386_32(symval, *ref);
            break;
        case R_386_PC32:
            *ref = DO_386_PC32(symval, *ref, (int) ref);
            break;
        default:
            kprintf("Unsupported relocation type %d.\n", ELF32_R_TYPE(relocation->r_info));
            return -1;
    }

    return symval;
}

static int ElfLoadStage2(const Elf32EHeader* const header)
{
    Elf32SectionHeader* const sectionHeader = GetElfSectionHeader(header);

    for(u32 i = 0; i < header->e_shnum; ++i)
    {
        Elf32SectionHeader* const section = &sectionHeader[i];

        if(section->sh_type == SHT_REL)
        {
            for(u32 j = 0; j < section->sh_size / section->sh_entsize; ++j)
            {
                Elf32Relocation* relocation = &((Elf32Relocation*) ((int) header + section->sh_offset))[j];
                int result = ElfPerformRelocation(header, relocation, section);

                if(result < 0)
                {
                    kprintf("Failed to reallocate symbol %u in section %u.\n", j, i);
                    return result;
                }
            }
        }
    }

    return 0;
}

static int ElfLoadExecStage2(const Elf32EHeader* const header) 
{
    Elf32SectionHeader* const sectionHeader = GetElfSectionHeader(header);

    for(u32 i = 0; i < header->e_shnum; ++i)
    {
        Elf32SectionHeader* const section = &sectionHeader[i];

        if(section->sh_type == SHT_PROGBITS)
        {
            const uSys currAddress = (uSys) header + section->sh_offset;

            if(currAddress == section->sh_addr)
            {
                continue;
            }

            memcpy((void*) section->sh_addr, (const void*) currAddress, section->sh_size);
        }
    }

    return 0;
}

static void* Elf32LoadFileExecutable(Elf32EHeader* const header)
{
    kprintf("Loading executable ELF.\n");

    int result = ElfLoadStage1(header);

    if(result < 0)
    {
        kprintf("Failed to load ELF file.\n");
        return nullptr;
    }

    result = ElfLoadExecStage2(header);

    if(result < 0)
    {
        kprintf("Failed to load ELF file.\n");
        return nullptr;
    }

    return (void*) header->e_entry;
}

static void* Elf32LoadFileRelocatable(Elf32EHeader* const header)
{
    kprintf("Loading relocatable ELF.\n");

    int result = ElfLoadStage1(header);

    if(result < 0)
    {
        kprintf("Failed to load ELF file.\n");
        return nullptr;
    }

    result = ElfLoadStage2(header);

    if(result < 0)
    {
        kprintf("Failed to load ELF file.\n");
        return nullptr;
    }

    return (void*) header->e_entry;
}

void* Elf32LoadFile(void* const file)
{
    Elf32EHeader* header = (Elf32EHeader*) file;

    if(!ElfCheckHeader(header) || !ElfIsSupported(header))
    {
        kprintf("ELF file cannot be loaded.\n");
        return nullptr;
    }

    switch(header->e_type)
    {
        case ET_EXEC: return Elf32LoadFileExecutable(header);
        case ET_REL:  return Elf32LoadFileRelocatable(header);
        default: return nullptr;
    }
}
