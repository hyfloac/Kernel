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
    DumpMemoryLayout();

    const KError_t pageInitRes = InitPageMap();
    kprintf("Page Init Res: %s\n", KernelLocalizeErrorCode(pageInitRes));

    SetupPaging32();

    kprintf("Print after enable paging!\n");

    if(1)
    {
        const KError_t registerKeyboardError = RegisterPS2KeyboardDriver();

        if(registerKeyboardError != KE_OK)
        {
            kprintf("Keyboard registration return (%d) %s.\n", registerKeyboardError, KernelLocalizeErrorCode(registerKeyboardError));
            kprintf("%s\n", KernelGetErrorMessageSafe(0));
        }
    }

    while(true) { }
}
