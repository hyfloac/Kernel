#pragma once

#include "kstdint.h"
#include "kassert.h"

#define PIC1_ADDRESS (0x20)
#define PIC2_ADDRESS (0xA0)

#define PIC1_COMMAND PIC1_ADDRESS
#define PIC2_COMMAND PIC2_ADDRESS

#define PIC1_DATA (PIC1_ADDRESS + 1)
#define PIC2_DATA (PIC2_ADDRESS + 1)

#define PIC_READ_IRR (0x0A) // OCW3 IRQ Ready Next CMD Read.
#define PIC_READ_ISR (0x0B) // OCW3 IRQ Service Next CMD Read.

#define PIC_EOI (0x20)

#ifdef __cplusplus
extern "C" {
#endif

enum ICW1_TriggerMode
{
    ICW1_EDGE_TRIGGER_MODE = 0,
    ICW1_LEVEL_TRIGGER_MODE = 1
};

enum ICW1_SuccessiveInterruptSize
{
    ICW1_SUCCESSIVE_INTERRUPT_8 = 0,
    ICW1_SUCCESSIVE_INTERRUPT_4 = 1
};

enum ICW1_Cascade
{
    ICW1_CASCADE_MODE = 0,
    ICW1_SINGLE_MODE = 1
};

enum ICW1_RequiresICW4
{
    ICW1_NEEDS_ICW4 = 0,
    ICW1_DOES_NOT_NEED_ICW4 = 1
};

typedef union
{
    struct
    {
        u8 _8085 : 3;
        u8 Issued : 1;
        u8 TriggerMode : 1;
        u8 SuccessiveInterruptSize : 1;
        u8 Cascade : 1;
        u8 RequiresICW4 : 1;
    };
    u8 icw1;
} ICW1;

typedef union
{
    struct
    {
        u8 BaseVectorAddress : 5;
        u8 Reserved : 3;
    };
    u8 icw2;
} ICW2;

enum ICW3_SlaveControllerActive
{
    ICW3_SLAVE_NOT_ATTACHED = 0,
    ICW3_SLAVE_ATTACHED = 1
};

typedef union
{
    u8 SlaveControllerActive;
    u8 icw3;
} ICW3;

enum ICW4_FullyNestedMode
{
    ICW4_NO_SPECIAL_FULLY_NESTED_MODE = 0,
    ICW4_SPECIAL_FULLY_NESTED_MODE = 1
};

enum ICW4_BufferMode
{
    ICW4_NON_BUFFERED = 0,
    ICW4_NON_BUFFERED_0 = 0,
    ICW4_NON_BUFFERED_1 = 1,
    ICW4_BUFFERED_SLAVE = 2,
    ICW4_BUFFERED_MASTER = 3,
};

#define ICW4_IS_NON_BUFFERED(_ICW4_BufferMode) ((_ICW4_BufferMode) == ICW4_NON_BUFFERED_0 || (_ICW4_BufferMode) == ICW4_NON_BUFFERED_1)

enum ICW4_EndOfInterruptMode
{
    ICW4_NORMAL_EOI_MODE = 0,
    ICW4_AUTO_EOI_MODE = 1
};

enum ICW4_80Mode
{
    ICW4_8085_MODE = 0,
    ICW4_8086_8088_MODE = 1
};

typedef union
{
    struct
    {
        u8 Reserved : 3; // 0
        u8 FullyNestedMode : 1;
        u8 BufferMode : 2;
        u8 EndOfInterruptMode : 1;
        u8 _80Mode : 1;
    };
    u8 icw4;
} ICW4;

typedef union
{
    struct
    {
        u8 ParallelPrinter : 1;
        u8 Diskette : 1;
        u8 FixedDisk : 1;
        u8 SerialPort1 : 1;
        u8 SerialPort2 : 1;
        u8 Video : 1;
        u8 KeyboardMouseRTC : 1;
        u8 Timer : 1;
    };
    u8 ocw1;
} OCW1_PIC1;

typedef union
{
    struct
    {
        u8 Reserved0 : 1;
        u8 FixedDisk : 1;
        u8 CoprocessorException : 1;
        u8 Mouse : 1;
        u8 Reserved1 : 1;
        u8 Reserved2 : 1;
        u8 RedirectCascade : 1;
        u8 RealTimeClock : 1;
    };
    u8 ocw1;
} OCW1_PIC2;

typedef union
{
    struct
    {
        OCW1_PIC2 pic2;
        OCW1_PIC1 pic1;
    };
    u16 ocw1;
} OCW1;

enum OCW2_Command
{
    OCW2_CLEAR_ROTATE_IN_AUTO_EOI_MODE = 0,
    OCW2_NON_SPECIFIC_EOI = 1,
    OCW2_NO_OPERATION = 2,
    OCW2_SPECIFIC_EOI = 3,
    OCW2_SET_ROTATE_IN_AUTO_EOI_MODE = 4,
    OCW2_ROTATE_ON_NON_SPECIFIC_EOI_COMMAND = 5,
    OCW2_SET_PRIORITY_COMMAND = 6,
    OCW2_ROTATE_ON_SPECIFIC_EOI_COMMAND = 7,
};

typedef union
{
    struct
    {
        u8 Command : 3;
        u8 Reserved0 : 1;  // 0
        u8 Reserved1 : 1;  // 0
        u8 IRQ : 3;
    };
    u8 ocw2;
} OCW2;

enum OCW3_MaskCommand
{
    OCW3_MASK_NO_OPERATION = 0,
    OCW3_MASK_NO_OPERATION_0 = 0,
    OCW3_MASK_NO_OPERATION_1 = 1,
    OCW3_RESET_SPECIAL_MASK = 2,
    OCW3_SET_SPECIAL_MASK = 3
};

#define OCW3_MASK_IS_NO_OPERATION(_OCW3_MaskCommand) ((_OCW3_MaskCommand) == OCW3_MASK_NO_OPERATION_0 || (_OCW3_MaskCommand) == OCW3_MASK_NO_OPERATION_1)

enum OCW3_PollCommand
{
    OCW3_NO_POLL_COMMAND = 0,
    OCW3_POLL_COMMAND = 1
};

enum OCW3_RegisterCommand
{
    OCW3_REGISTER_NO_OPERATION = 0,
    OCW3_REGISTER_NO_OPERATION_0 = 0,
    OCW3_REGISTER_NO_OPERATION_1 = 1,
    OCW3_READ_REQUEST_REGISTER = 2,
    OCW3_READ_IN_SERVICE_REGISTER = 3
};

#define OCW3_REGISTER_IS_NO_OPERATION(_OCW3_RegisterCommand) ((_OCW3_RegisterCommand) == OCW3_REGISTER_NO_OPERATION_0 || (_OCW3_RegisterCommand) == OCW3_REGISTER_NO_OPERATION_1)

typedef union
{
    struct
    {
        u8 Reserved0 : 1; // 0
        u8 MaskCommand : 2;
        u8 Reserved1 : 1; // 0
        u8 Reserved2 : 1; // 1
        u8 PollCommand : 1;
        u8 RegisterCommand : 2;
    };
    u8 ocw3;
} OCW3;

enum OCW3_RequestRegister
{
    OCW3_NO_ACTIVE_REQUEST = 0,
    OCW3_ACTIVE_REQUEST = 1
};

enum OCW3_InServiceRegister
{
    OCW3_NOT_BEING_SERVICED = 0,
    OCW3_BEING_SERVICED = 1
};

typedef union
{
    u8 RequestRegister;
    u8 InServiceRegister;
    u8 ocw3;
} OCW3_In;

STATIC_ASSERT(sizeof(ICW1) == 1, "ICW1 is not 1 byte in size.");
STATIC_ASSERT(sizeof(ICW2) == 1, "ICW2 is not 1 byte in size.");
STATIC_ASSERT(sizeof(ICW3) == 1, "ICW3 is not 1 byte in size.");
STATIC_ASSERT(sizeof(ICW4) == 1, "ICW4 is not 1 byte in size.");

STATIC_ASSERT(sizeof(OCW1_PIC1) == 1, "OCW1_PIC1 is not 1 byte in size.");
STATIC_ASSERT(sizeof(OCW1_PIC2) == 1, "OCW1_PIC2 is not 1 byte in size.");
STATIC_ASSERT(sizeof(OCW1) == 2, "OCW1 is not 2 bytes in size.");
STATIC_ASSERT(sizeof(OCW2) == 1, "OCW2 is not 1 byte in size.");
STATIC_ASSERT(sizeof(OCW3) == 1, "OCW3 is not 1 byte in size.");
STATIC_ASSERT(sizeof(OCW3_In) == 1, "OCW3_In is not 1 byte in size.");

#ifdef __cplusplus
} /* extern "C" */
#endif
