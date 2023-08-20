#include "interrupt.h"
#include "segment.h"
#include "console.h"
#include "itoa.h"
#include "pic.h"
#include "io.h"
#include "kprintf.h"
#include "ps2.h"
#include "keyboard.h"

#define IDT_MAX_DESCRIPTORS 256

_Static_assert(sizeof(IDTEntry32) == 8, "IDTEntry32 is not the correct size.");
_Static_assert(sizeof(IDTR32) == 6, "IDTR32 is not the correct size.");

__attribute__((aligned(64))) static IDTEntry32 idt32[IDT_MAX_DESCRIPTORS];
// __attribute__((aligned(64))) static IDTEntry64 idt64[IDT_MAX_DESCRIPTORS];

static IDTR32 idtr32;
// static IDTR64 idtr64;

extern void* isr_stub_table32[];
extern void i_print();
// extern void* ist_stub_table64[];

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionDivideBy0(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionDebug(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionNMI(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionBreakpoint(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionOverflow(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionBoundRangeExceeded(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionUndefinedOpcode(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionNoCoprocessor(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionDoubleFault(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionCoProcessorOverrun(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionInvalidTss(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionSegmentNotPresent(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionStackSegmentFault(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionGeneneralProtection(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionPageFault(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionFloatingPointError(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionAlignmentCheck(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionMachineCheck(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionSimdFloatingPointError(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionVirtualizationException(IsrExceptionFrame* const frame);
static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionControlProtectionException(IsrExceptionFrame* const frame);

static FASTCALL_GCC void FASTCALL_MSVC IsrPicPit();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicKeyboard();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicCom2();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicCom1();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicLpt2();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicFloppy();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicLpt1();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicCmos();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicMouse();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicFpu();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicPrimaryAta();
static FASTCALL_GCC void FASTCALL_MSVC IsrPicSecondaryAta();

void IDTSetDescriptor32(const u8 vector, u32 isr, const u8 attributes)
{
    idt32[vector] = CreateIDTEntry32(isr, CODE_SEG, attributes);
}

// void IDTSetDescriptor64(const u8 vector, u64 isr, const u8 attributes)
// {
//     idt64[vector] = CreateIDTEntry64(isr, 0, 0, attributes);
// }

void InitIDT32()
{
    idtr32 = CreateIDTR32(sizeof(IDTEntry32) * IDT_MAX_DESCRIPTORS - 1, (u32) idt32);

    for(u32 vector = 0; vector < 32; ++vector)
    {
        IDTSetDescriptor32((u8) vector, (u32) isr_stub_table32[vector], 0x8E);
    }

    for(u32 vector = 32; vector < 48; ++vector)
    {
        IDTSetDescriptor32((u8) vector, (u32) isr_stub_table32[vector], 0x8E);
    }

    for(u32 vector = 48; vector < IDT_MAX_DESCRIPTORS; ++vector)
    {
        IDTSetDescriptor32((u8) vector, 0, 0);
    }

    IDTSetDescriptor32(0x80, (u32) i_print, 0x8E);
}

void LoadIDT32()
{
    load_idt32(&idtr32);
}

static FASTCALL_GCC void FASTCALL_MSVC DumpCPUState(const IsrExceptionFrame* const frame);

FASTCALL_GCC void FASTCALL_MSVC isr_handler(const u32 vector)
{
    if(vector >= 0x20 && vector < 0x30)
    {
        // Handle spurious IRQ on Master controller.
        if(vector == 0x27)
        {
            const u16 isr = PICGetISR();
            if(!(isr & 0x80))
            {
                return;
            }
        }
        // Handle spurious IRQ on Slave controller.
        else if(vector == 0x2F)
        {
            const u16 isr = PICGetISR();
            if(!(isr & 0x08))
            {
                PICSendEOI(0x02);
                return;
            }
        }

        switch(vector - 0x20)
        {
            case 0x0: IsrPicPit(); break;
            case 0x1: IsrPicKeyboard(); break;
            case 0x2: break; // The cascade IRQ, never sent.
            case 0x3: IsrPicCom2(); break;
            case 0x4: IsrPicCom1(); break;
            case 0x5: IsrPicLpt2(); break;
            case 0x6: IsrPicFloppy(); break;
            case 0x7: IsrPicLpt1(); break;
            case 0x8: IsrPicCmos(); break;
            case 0x9: break; // Free for peripherals / legacy SCSI / NIC
            case 0xA: break; // Free for peripherals / SCSI / NIC
            case 0xB: break; // Free for peripherals / SCSI / NIC
            case 0xC: IsrPicMouse(); break;
            case 0xD: IsrPicFpu(); break;
            case 0xE: IsrPicPrimaryAta(); break;
            case 0xF: IsrPicSecondaryAta(); break;
        }

        PICSendEOI(vector - 0x20);
    }
}

FASTCALL_GCC void FASTCALL_MSVC isr_exception_handler(IsrExceptionFrame* const frame)
{
    DumpCPUState(frame);

    switch(frame->Vector)
    {
        case 0:  IsrExceptionDivideBy0(frame); break;
        case 1:  IsrExceptionDebug(frame); break;
        case 2:  IsrExceptionNMI(frame); break;
        case 3:  IsrExceptionBreakpoint(frame); break;
        case 4:  IsrExceptionOverflow(frame); break;
        case 5:  IsrExceptionBoundRangeExceeded(frame); break;
        case 6:  IsrExceptionUndefinedOpcode(frame); break;
        case 7:  IsrExceptionNoCoprocessor(frame); break;
        case 8:  IsrExceptionDoubleFault(frame); break;
        case 9:  IsrExceptionCoProcessorOverrun(frame); break;
        case 10: IsrExceptionInvalidTss(frame); break;
        case 11: IsrExceptionSegmentNotPresent(frame); break;
        case 12: IsrExceptionStackSegmentFault(frame); break;
        case 13: IsrExceptionGeneneralProtection(frame); break;
        case 14: IsrExceptionPageFault(frame); break;
        case 16: IsrExceptionFloatingPointError(frame); break;
        case 17: IsrExceptionAlignmentCheck(frame); break;
        case 18: IsrExceptionMachineCheck(frame); break;
        case 19: IsrExceptionSimdFloatingPointError(frame); break;
        case 20: IsrExceptionVirtualizationException(frame); break;
        case 21: IsrExceptionControlProtectionException(frame); break;
        default: break;
    }

    ConSwapBuffers();
}

static FASTCALL_GCC void FASTCALL_MSVC DumpCPUState(const IsrExceptionFrame* const frame)
{
    ConClearScreen();
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';

    u32 yCoord = 0;

    ConSetCursorCoord(0, yCoord++);
    ConWriteStringColor("Exception Triggered: ", ConColor_Black, ConColor_White);
    Xtoap(frame->Vector, buf + 2);
    ConWriteStringColor(buf, ConColor_Black, ConColor_White);

    {
        char bBuf[33];
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("Error Code", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        btoap(frame->ErrorCode, bBuf);
        ConWriteStringColor(bBuf, ConColor_Black, ConColor_Cyan);
    }

    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("eax", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->eax, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);

        ConWriteStringColor(" ebx", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->ebx, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);
    }
    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("ecx", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->ecx, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);

        ConWriteStringColor(" edx", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->edx, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);
    }
    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("esi", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->esi, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);

        ConWriteStringColor(" edi", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->edi, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);
    }
    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("esp", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->esp, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);

        ConWriteStringColor(" ebp", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->ebp, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);
    }
    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("eip", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->eip, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);


        char bBuf[33];
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("eflags", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        btoap(frame->eflags, bBuf);
        ConWriteStringColor(bBuf, ConColor_Black, ConColor_Cyan);
    }

    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("cr4", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->cr4, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);

        ConWriteStringColor(" cr3", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->cr3, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);
    }

    {
        ConSetCursorCoord(0, yCoord++);
        ConWriteStringColor("cr2", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->cr2, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);

        ConWriteStringColor(" cr0", ConColor_Black, ConColor_LightBlue);
        ConWriteStringColor("=", ConColor_Black, ConColor_LightGray);
        Xtoap(frame->cr0, buf + 2);
        ConWriteStringColor(buf, ConColor_Black, ConColor_Cyan);
    }
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionDivideBy0(IsrExceptionFrame* const frame)
{
    (void) frame;

    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Divide By 0", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionDebug(IsrExceptionFrame* const frame)
{
    (void) frame;

    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Debug", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionNMI(IsrExceptionFrame* const frame)
{
    (void) frame;

    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Non-Maskable Interrupt", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionBreakpoint(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Breakpoint", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionOverflow(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Integer Overflow", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionBoundRangeExceeded(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Bound Range Exceeded", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionUndefinedOpcode(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Undefined Opcode", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionNoCoprocessor(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("No Math CoProcessor", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionDoubleFault(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Double Fault", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionCoProcessorOverrun(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("CoProcessor Overrun", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionInvalidTss(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Invalid Task Switch Segment", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionSegmentNotPresent(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Segment Not Present", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionStackSegmentFault(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Stack Segment Fault", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionGeneneralProtection(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("General Protection", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionPageFault(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Page Fault", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionFloatingPointError(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Floating-Point Error", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionAlignmentCheck(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Alignment Check", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionMachineCheck(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Machine Check", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionSimdFloatingPointError(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("SIMD Floating-Point Error", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionVirtualizationException(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Virtualization Exception", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrExceptionControlProtectionException(IsrExceptionFrame* const frame)
{
    (void) frame;
    
    ConSetCursorCoord(0, 16);
    ConWriteStringColor("Control Protection Exception", ConColor_Black, ConColor_Red);
}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicPit()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicKeyboard()
{
#if 1
    KeyboardNotifyKeyEvent(GetPS2KeyboardPDO());
#else
    const u32 keyCode = KeyboardReadKeyCode(GetPS2KeyboardPDO());

    if(KEY_IS_PRESSED(keyCode))
    {
        if(KEY_CODE(keyCode) == KEY_CODE_BACKSPACE)
        {
            ConBackspace();
        }
        else if(KEY_CODE(keyCode) == KEY_CODE_ENTER)
        {
            ConNewLine();
        }
        else
        {
            const u32 unicode = CodepointFromKeyCode(GetPS2KeyboardPDO(), keyCode, KEYBOARD_LAYOUT_US_QWERTY);
            if(unicode)
            {
                ConWriteChar(unicode);
            }
        }
    }

    ConSwapBuffers();
#endif
}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicCom2()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicCom1()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicLpt2()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicFloppy()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicLpt1()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicCmos()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicMouse()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicFpu()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicPrimaryAta()
{

}

static FASTCALL_GCC void FASTCALL_MSVC IsrPicSecondaryAta()
{

}


// void InitIDT64()
// {
//     idtr64 = CreateIDTR64(sizeof(IDTEntry64) * IDT_MAX_DESCRIPTORS - 1, &idt64[0]);

//     for(u32 vector = 0; vector < IDT_MAX_DESCRIPTORS; ++vector)
//     {
//         IDTSetDescriptor64((u8) vector, ist_stub_table64[vector], 0x8E);
//     }

//     load_idt64(&idtr64);
// }
