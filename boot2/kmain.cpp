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


    PICRemap(0x20, 0x28);
    PICSetMask(0);
    PICSetMask(1);
    PICSetMask(2);
    PICSetMask(3);
    PICSetMask(4);
    PICSetMask(5);
    PICSetMask(6);
    PICSetMask(7);
    InitIDT32();
    LoadIDT32();

    // const char* message = "Hello, World!";
    
    // ConSetCursorCoord(0, 1);
    // ConWriteString(message);
    // ConSwapBuffers();
    // ConSetCursorCoord(0, 2);

    CheckMemoryLayout();
    DumpMemoryLayout();

    const KError_t pageInitRes = InitPageMap();
    kprintf("Page Init Res: %s\n", KernelLocalizeErrorCode(pageInitRes));

    SetupPaging32();

    kprintf("Print after enable paging!\n");
}
