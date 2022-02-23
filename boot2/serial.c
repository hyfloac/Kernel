#include "kstdint.h"
#include "serial.h"
#include "io.h"

#define COM1_PORT (0x03F8)
#define DATA_REGISTER (0)
#define INTERRUPT_ENABLE_REGISTER (1)
#define DIVISOR_LSB (0)
#define DIVISOR_MSB (1)
#define INTERRUPT_IDENT (2)
#define LINE_CONTROL_REGISTER (3)
#define MODEM_CONTROL_REGISTER (4)
#define LINE_STATUS_REGISTER (5)
#define MODEM_STATUS_REGISTER (6)
#define SCRATCH_REGISTER (7)

#define ENABLE_DLAB (0x80)
#define LOOPBACK_MODE (0x1E)
#define NORMAL_MODE (0x0F)
#define SERIAL_8N1 (0x03)
#define ENABLE_FIFO (0xC7)
#define ENABLE_IRQ (0x0B)
#define READBACK_TEST_VAL (0x69)

static u8* SerialReadBuffer = NULL;
static uSys SerialReadSize = 0;
static uSys SerialReadReadIndex = -1;
static uSys SerialReadWriteIndex = -1;

static u8* SerialWriteBuffer = NULL;
static uSys SerialWriteSize = 0;
static uSys SerialWriteReadIndex = -1;
static uSys SerialWriteWriteIndex = -1;

FASTCALL_GCC int FASTCALL_MSVC SerialInit(void* const readBuffer, const uSys readBufferSize, void* const writeBuffer, const uSys writeBufferSize)
{
    out8(COM1_PORT + INTERRUPT_ENABLE_REGISTER, 0x00);          // Disable all interrupts
    out8(COM1_PORT + LINE_CONTROL_REGISTER, ENABLE_DLAB);       // Enable DLAB (set baud rate)
    out8(COM1_PORT + DIVISOR_LSB, 0x03);                        // Set divisior to 3 (38400 Baud)
    out8(COM1_PORT + DIVISOR_MSB, 0x00);    
    out8(COM1_PORT + LINE_CONTROL_REGISTER, SERIAL_8N1);        // 8 Char Bits, No Parity, One Stop Bit
    out8(COM1_PORT + INTERRUPT_IDENT, ENABLE_FIFO);             // Enable FIFO, clear, 14 byte threshold
    out8(COM1_PORT + MODEM_CONTROL_REGISTER, ENABLE_IRQ);       // IRQs enabled, RTS/DSR set
    out8(COM1_PORT + MODEM_CONTROL_REGISTER, LOOPBACK_MODE);    // Enable loopback mode for validation testing
    out8(COM1_PORT + DATA_REGISTER, READBACK_TEST_VAL);         // Send 0x69 as test readback.

    if(in8(COM1_PORT + DATA_REGISTER) != READBACK_TEST_VAL)
    {
        KernelSetErrorMessage("COM1 is corrupt, could not correctly perform loopback test.", 0);
        return KE_FAIL;
    }

    out8(COM1_PORT + MODEM_CONTROL_REGISTER, NORMAL_MODE);      // Set to normal mode now that everything has been validated    

    SerialReadBuffer = readBuffer;
    SerialReadSize = readBufferSize;
    SerialReadReadIndex = -1;
    SerialReadWriteIndex = 0;
    
    SerialWriteBuffer = writeBuffer;
    SerialWriteSize = writeBufferSize;
    SerialWriteReadIndex = -1;
    SerialWriteWriteIndex = 0;

    KernelClearErrorMessage();
    return KE_OK;
}

static int SerialAttemptWrite()
{
    if(SerialWriteReadIndex != SerialWriteWriteIndex - 1)
    {
        if(SerialIsWriteReady())
        {
            const u8 c = SerialWriteBuffer[++SerialWriteReadIndex];
            if(SerialWriteWriteIndex + 1 == SerialWriteSize)
            {
                SerialWriteWriteIndex = -1;
            }
            out8(COM1_PORT + DATA_REGISTER, c);
            return 1;
        }
    }
    return 0;
}

void SerialUpdateTick()
{
    if(SerialIsReadReady())
    {
        if(SerialReadWriteIndex != SerialReadReadIndex - 1)
        {
            const u8 c = in8(COM1_PORT + DATA_REGISTER);
            SerialReadBuffer[SerialReadWriteIndex++] = c;
            if(SerialReadWriteIndex == SerialReadSize)
            {
                SerialReadWriteIndex = 0;
            }
        }
    }

    SerialAttemptWrite();    
}

int SerialIsReadReady()
{
    return in8(COM1_PORT + LINE_STATUS_REGISTER) & 0x01;
}

int SerialIsWriteReady()
{
    return in8(COM1_PORT + LINE_STATUS_REGISTER) & 0x20;
}

char SerialReadCharBlocking()
{
    if(SerialReadReadIndex - 1 != SerialReadWriteIndex)
    {
        const u8 c = SerialReadBuffer[++SerialReadReadIndex];
        if(SerialReadReadIndex + 1 == SerialReadSize)
        {
            SerialReadReadIndex = -1;
        }
        return c;
    }

    while(!SerialIsReadReady());

    return in8(COM1_PORT + DATA_REGISTER);
}

FASTCALL_GCC int SerialReadCharNonBlocking(char* const c)
{
    if(SerialReadReadIndex - 1 != SerialReadWriteIndex)
    {
        *c = SerialReadBuffer[++SerialReadReadIndex];
        if(SerialReadReadIndex + 1 == SerialReadSize)
        {
            SerialReadReadIndex = -1;
        }
        return 1;
    }

    if(!SerialIsReadReady())
    {
        return 0;
    }

    *c = in8(COM1_PORT + DATA_REGISTER);
    return 1;
}

FASTCALL_GCC int FASTCALL_MSVC SerialWriteCharBlocking(const char c)
{
    if(SerialWriteCharNonBlocking(c))
    {
        return 1;
    }

    while(!SerialAttemptWrite());

    return SerialWriteCharNonBlocking(c);
}

FASTCALL_GCC int FASTCALL_MSVC SerialWriteCharNonBlocking(const char c)
{
    if(SerialWriteWriteIndex != SerialWriteReadIndex - 1)
    {
        SerialWriteBuffer[SerialWriteWriteIndex++] = c;
        if(SerialWriteWriteIndex == SerialWriteSize)
        {
            SerialWriteWriteIndex = 0;
        }
        return 1;
    }
    return 0;
}
