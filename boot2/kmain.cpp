#include "console.h"
#include "interrupt.h"
#include "kstdint.h"
#include "callspec.h"
#include "checkmem.h"
#include "pic.h"
#include "page_map.h"
#include "kprintf.h"
#include "serial.h"
#include "kerror.h"
#include "paging.h"
#include "ps2.h"
#include "kalloc.h"
#include "keyboard.h"
#include "pool_allocator.h"
#include "command_line.h"
#include "mdl.h"
#include "avl_tree.h"
#include "kstring.h"
#include "memcpy.h"

char serialReadBuffer[1024];
char serialWriteBuffer[1024];

struct Test
{
    int X;
    EmbeddedAvlTree Tree;
};

static i32 AvlComparator(EmbeddedAvlTree* const branch, EmbeddedAvlTree* const node) noexcept
{
    return EMBEDDED_OBJECT(node, Test, Tree)->X - EMBEDDED_OBJECT(branch, Test, Tree)->X;
}

static void PrintTestTree(EmbeddedAvlTree* const node, const char* const prefix)
{
    Test* t = EMBEDDED_OBJECT(node, Test, Tree);

    kprintf("%s: Tree x%d\n", prefix, t->X);

    const int prefixLen = StrLen(prefix);
    char* newPrefix = static_cast<char*>(kalloc(sizeof(char) * (prefixLen + 2)));
    memcpy(newPrefix, prefix, sizeof(char) * prefixLen);
    newPrefix[prefixLen + 1] = '\0';

    if(node->Left)
    {
        newPrefix[prefixLen] = 'L';
        PrintTestTree(node->Left, newPrefix);
    }

    if(node->Right)
    {
        newPrefix[prefixLen] = 'R';
        PrintTestTree(node->Right, newPrefix);
    }

    kfree(newPrefix);
}

extern "C" void kmain()
{
    PICDisable();

    InitIDT32();
    LoadIDT32();

    PICRemap(0x20, 0x28);
    PICSetMask(0x0);
    PICClearMask(0x1);
    PICClearMask(0x2);
    PICSetMask(0x3);
    PICSetMask(0x4);
    PICSetMask(0x5);
    PICSetMask(0x6);
    PICSetMask(0x7);
    PICSetMask(0x8);
    PICSetMask(0x9);
    PICSetMask(0xA);
    PICSetMask(0xB);
    PICSetMask(0xC);
    PICSetMask(0xD);
    PICSetMask(0xE);
    PICSetMask(0xF);

    ConInit();

    // ConWriteString("test");
    // ConSwapBuffers();
    // return;

    {
        const KError_t serialError = SerialInit(serialReadBuffer, sizeof(serialReadBuffer), serialWriteBuffer, sizeof(serialWriteBuffer));
        if(serialError)
        {
            const char* const errorLocalized = KernelLocalizeErrorCode(serialError);
            const char* const errorMessage = KernelGetErrorMessage(0);

            kprintf("Failed to intialize serial [%s]: %s\n", errorLocalized, errorMessage);
        }
    }
    SerialWriteCharNonBlocking('a');
    SerialWriteCharNonBlocking('\n');
    SerialUpdateTick();
    SerialUpdateTick();
    SerialUpdateTick();


    // const char* message = "Hello, World!";
    
    // ConSetCursorCoord(0, 1);
    // ConWriteString(message);
    // ConSwapBuffers();
    // ConSetCursorCoord(0, 2);

    CheckMemoryLayout();
    // DumpMemoryLayout();

    // return;

    {
        const KError_t pageInitRes = InitPageMap();
        if(!KE_IS_OK(pageInitRes))
        {
            kprintf("Page Init Res: (%d) %s\n", pageInitRes, KernelLocalizeErrorCode(pageInitRes));
            kprintf("%s\n", KernelGetErrorMessageSafe(0));
        }
    }

    SetupPaging32();

    kprintf("Print after enable paging!\n");

    InitKAlloc();

    InitPoolAllocator();

    InitKeyboard();

    if(1)
    {
        KError_t keyboardError = RegisterPS2KeyboardDriver();

        if(!KE_IS_OK(keyboardError))
        {
            kprintf("Keyboard registration return (%d) %s.\n", keyboardError, KernelLocalizeErrorCode(keyboardError));
            kprintf("%s\n", KernelGetErrorMessageSafe(0));
        }
        else
        {
            keyboardError = GetPS2KeyboardPDO()->ManagerAddDevice(GetPS2KeyboardPDO()->Driver);
            if(!KE_IS_OK(keyboardError))
            {
                kprintf("Keyboard AddDevice return (%d) %s.\n", keyboardError, KernelLocalizeErrorCode(keyboardError));
                kprintf("%s\n", KernelGetErrorMessageSafe(0));
            }
            else
            {
                keyboardError = GetPS2KeyboardPDO()->ManagerStartDevice(GetPS2KeyboardPDO()->Driver);

                if(!KE_IS_OK(keyboardError))
                {
                    kprintf("Keyboard StartDevice return (%d) %s.\n", keyboardError, KernelLocalizeErrorCode(keyboardError));
                    kprintf("%s\n", KernelGetErrorMessageSafe(0));
                }
            }
        }
    }
    
    if(0)
    {
        uSys tagCount = 0;
        KError_t error = EnumTaggedAllocationInfos(&tagCount, NULL);
        if(!KE_IS_OK(error))
        {
            kprintf("Enum Tags count return (%d) %s.\n", error, KernelLocalizeErrorCode(error));
            kprintf("%s\n", KernelGetErrorMessageSafe(0));
        }

        TaggedAllocationsInfo* infos = (TaggedAllocationsInfo*) kalloc(tagCount * sizeof(TaggedAllocationsInfo));
        error = EnumTaggedAllocationInfos(&tagCount, infos);

        if(!KE_IS_OK(error))
        {
            kprintf("Enum Tags return (%d) %s.\n", error, KernelLocalizeErrorCode(error));
            kprintf("%s\n", KernelGetErrorMessageSafe(0));
        }

        for(uSys i = 0; i < tagCount; ++i)
        {
            u32 tagString[2] = { infos[i].Tag, 0 };

            kprintf("[%s] <%0X>: %u Current Allocs, %u Frees, %u Bytes\n", tagString, infos[i].Tag, infos[i].CurrentAllocationCount, infos[i].TotalFrees, infos[i].TotalAllocationSizeLow);
        }

        kfree(infos);
    }

    if(0)
    {
        const uSys length = 23895;
        // const uSys length = 5000;
        void* test = kalloc(length);

        MDL* mdl = MmAllocMdl(test, length);

        kprintf("MDL: BlockCount: %d, VirtualAddress: %^p\n", (u32) mdl->BlockCount, mdl->VirtualAddress);

        MDLBlock* currentBlock = (MDLBlock*) (mdl + 1);

        for(u64 i = 0; i < mdl->BlockCount; ++i, ++currentBlock)
        {
            kprintf("  Block %d, Start: 0x%X%X, Count: 0x%X%X\n", (u32) i, (u32) (currentBlock->StartPage >> 32), (u32) currentBlock->StartPage, (u32) (currentBlock->PageCount >> 32), (u32) currentBlock->PageCount);
        }

        MmFreeMdl(mdl);
        kfree(test);
    }

    if(0)
    {
        Test x0;
        x0.X = 0;
        AvlTree_AvlTree(&x0.Tree);

        EmbeddedAvlTree* pRoot = &x0.Tree;

        Test x1;
        x1.X = 1;
        AvlTree_AvlTree(&x1.Tree);

        Test x2;
        x2.X = 2;
        AvlTree_AvlTree(&x2.Tree);

        Test x3;
        x3.X = 3;
        AvlTree_AvlTree(&x3.Tree);

        // pRoot = AvlTree_Insert(pRoot, &x0.Tree, AvlComparator);
        pRoot = AvlTree_Insert(pRoot, &x1.Tree, AvlComparator);
        pRoot = AvlTree_Insert(pRoot, &x2.Tree, AvlComparator);
        pRoot = AvlTree_Insert(pRoot, &x3.Tree, AvlComparator);
        PrintTestTree(pRoot, "T");

    }

    {
        const KError_t error = InitCommandLine();

        if(!KE_IS_OK(error))
        {
            kprintf("Command line init return (%d) %s.\n", error, KernelLocalizeErrorCode(error));
            kprintf("%s\n", KernelGetErrorMessageSafe(0));
        }
    }

    while(true) { }
}
