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

char serialReadBuffer[1024];
char serialWriteBuffer[1024];

extern "C" void kmain()
{
    ConInit();

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


    // const char* message = "Hello, World!";
    
    // ConSetCursorCoord(0, 1);
    // ConWriteString(message);
    // ConSwapBuffers();
    // ConSetCursorCoord(0, 2);

    CheckMemoryLayout();
    // DumpMemoryLayout();

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
    
    if(1)
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
