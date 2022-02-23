#include "pic.h"
#include "pic.private.h"
#include "io.h"

FASTCALL_GCC void FASTCALL_MSVC PICRemap(const u8 offset1, const u8 offset2)
{
    const u8 pic1Mask = in8(PIC1_DATA);
    const u8 pic2Mask = in8(PIC2_DATA);

    ICW1 icw1Init;
    icw1Init.icw1 = 0;
    icw1Init.Issued = 1;
    icw1Init.TriggerMode = ICW1_EDGE_TRIGGER_MODE;
    icw1Init.SuccessiveInterruptSize = ICW1_SUCCESSIVE_INTERRUPT_8;
    icw1Init.Cascade = ICW1_CASCADE_MODE;
    icw1Init.RequiresICW4 = ICW1_NEEDS_ICW4;

    out8(PIC1_COMMAND, icw1Init.icw1); // Start initialization sequence in cascade mode.
    io_wait();
    out8(PIC2_COMMAND, icw1Init.icw1);
    io_wait();

    out8(PIC1_DATA, offset1); // ICW2: Master PIC Vector Offset.
    io_wait();
    out8(PIC2_DATA, offset2); // ICW2: Slave PIC Vector Offset.
    io_wait();

    out8(PIC1_DATA, 4); // ICW3: Tell the Master PIC that there is a slave PIC at IRQ2 (0000 0100).
    io_wait();
    out8(PIC2_DATA, 2); // ICW3: Tell the Slave PIC its cascade identity (0000 0010).
    io_wait();

    ICW4 icw4Init;
    icw4Init.icw4 = 0;
    icw4Init.FullyNestedMode = ICW4_NO_SPECIAL_FULLY_NESTED_MODE;
    icw4Init.BufferMode = ICW4_NON_BUFFERED;
    icw4Init.EndOfInterruptMode = ICW4_NORMAL_EOI_MODE;
    icw4Init._80Mode = ICW4_8086_8088_MODE;

    out8(PIC1_DATA, icw4Init.icw4);
    io_wait();
    out8(PIC2_DATA, icw4Init.icw4);
    io_wait();

    out8(PIC1_DATA, pic1Mask);
    out8(PIC2_DATA, pic2Mask);
}

void PICDisable()
{
    out8(PIC1_DATA, 0xFF);
    out8(PIC2_DATA, 0xFF);
}

FASTCALL_GCC void FASTCALL_MSVC PICSendEOI(const u8 irq)
{
    if(irq >= 8)
    {
        out8(PIC2_COMMAND, PIC_EOI);
    }

    out8(PIC1_COMMAND, PIC_EOI);
}

FASTCALL_GCC void FASTCALL_MSVC PICSetMask(u8 irqLine)
{
    u16 port;

    if(irqLine < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irqLine -= 8;
    }

    const u8 value = in8(port) | (1 << irqLine);
    out8(port, value);
}

FASTCALL_GCC void FASTCALL_MSVC PICClearMask(u8 irqLine)
{
    u16 port;

    if(irqLine < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irqLine -= 8;
    }

    const u8 value = in8(port) & ~(1 << irqLine);
    out8(port, value);
}

FASTCALL_GCC static u16 FASTCALL_MSVC PICGetIRQReg(const u8 ocw3)
{
    //   OCW3 to PIC COMMAND to get register values. PIC2 is
    // chained, and represents IRQ's 8-15. PIC1 is IRQ's 0-7, with
    // 2 being the chain.
    out8(PIC1_COMMAND, ocw3);
    out8(PIC2_COMMAND, ocw3);
    return (in8(PIC2_COMMAND) << 8) | in8(PIC1_COMMAND);
}

u16 PICGetIRR()
{
    OCW3 ocw3;
    ocw3.ocw3 = 0;
    ocw3.MaskCommand = OCW3_MASK_NO_OPERATION;
    ocw3.Reserved2 = 1;
    ocw3.PollCommand = OCW3_NO_POLL_COMMAND;
    ocw3.RegisterCommand = OCW3_READ_REQUEST_REGISTER;

    return PICGetIRQReg(ocw3.ocw3);
}

u16 PICGetISR()
{
    OCW3 ocw3;
    ocw3.ocw3 = 0;
    ocw3.MaskCommand = OCW3_MASK_NO_OPERATION;
    ocw3.Reserved2 = 1;
    ocw3.PollCommand = OCW3_NO_POLL_COMMAND;
    ocw3.RegisterCommand = OCW3_READ_IN_SERVICE_REGISTER;

    return PICGetIRQReg(ocw3.ocw3);
}
