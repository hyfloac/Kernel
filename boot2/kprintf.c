#include "kprintf.h"
#include "console.h"
#include "kstring.h"
#include "itoa.h"

FASTCALL_GCC static int FASTCALL_MSVC internal_kvprintf(const char* format, va_list vList);
FASTCALL_GCC static int FASTCALL_MSVC internal_kvsnprintf(char* buffer, u32 bufferSize, const char* format, va_list vList);

FASTCALL_GCC int FASTCALL_MSVC kvprintf(const char* const format, va_list vList)
{
    if(!format)
    {
        return -1;
    }

    return internal_kvprintf(format, vList);
}

FASTCALL_GCC int FASTCALL_MSVC kvsnprintf(char* const buffer, const u32 bufferSize, const char* const format, va_list vList)
{
    if(!format)
    {
        return -1;
    }

    if(buffer && !bufferSize)
    {
        return -2;
    }

    return internal_kvsnprintf(buffer, bufferSize, format, vList);
}

int kprintf(const char* const format, ...)
{
    if(!format)
    {
        return -1;
    }

    va_list vList;
    va_start(vList, format);
    
    const int ret = internal_kvprintf(format, vList);

    va_end(vList);

    return ret;
}

int ksnprintf(char* const buffer, const u32 bufferSize, const char* const format, ...)
{
    if(!format)
    {
        return -1;
    }

    if(buffer && !bufferSize)
    {
        return -2;
    }

    va_list vList;
    va_start(vList, format);
    
    const int ret = internal_kvsnprintf(buffer, bufferSize, format, vList);

    va_end(vList);

    return ret;
}

typedef enum
{
    FormatStage_None = 0,
    FormatStage_BehaviorFlags,
    FormatStage_FieldWidth,
    FormatStage_Precision,
    FormatStage_Length,
    FormatStage_ColorBackground,
    FormatStage_ColorForeground
} FormatStage;

typedef union
{
    struct
    {
        u32 LeftJustify : 1;
        u32 ForceSign : 1;
        u32 SpaceSign : 1;
        u32 AlternativeForm1 : 1;
        u32 AlternativeForm2 : 1;
        u32 PadZeros : 1;
        u32 IsSigned : 1;
    };
    u32 Packed;
} PrintfFlags;

static char GetDigitCharU(const i32 digit)
{
    switch(digit)
    {
        case 0x0: return '0';
        case 0x1: return '1';
        case 0x2: return '2';
        case 0x3: return '3';
        case 0x4: return '4';
        case 0x5: return '5';
        case 0x6: return '6';
        case 0x7: return '7';
        case 0x8: return '8';
        case 0x9: return '9';
        // case 0xA: return 'A';
        // case 0xB: return 'B';
        // case 0xC: return 'C';
        // case 0xD: return 'D';
        // case 0xE: return 'E';
        // case 0xF: return 'F';
        default:  return '?';
    }
}

// static char GetDigitCharL(const i32 digit)
// {
//     switch(digit)
//     {
//         case 0x0: return '0';
//         case 0x1: return '1';
//         case 0x2: return '2';
//         case 0x3: return '3';
//         case 0x4: return '4';
//         case 0x5: return '5';
//         case 0x6: return '6';
//         case 0x7: return '7';
//         case 0x8: return '8';
//         case 0x9: return '9';
//         case 0xA: return 'a';
//         case 0xB: return 'b';
//         case 0xC: return 'c';
//         case 0xD: return 'd';
//         case 0xE: return 'e';
//         case 0xF: return 'f';
//         default:  return '?';
//     }
// }

static u32 PadIntStr(char* const buffer, u32 writeIndex, const u32 padSize, const PrintfFlags flags, const u32 isNegative)
{
    if(flags.PadZeros)
    {
        while(writeIndex < padSize)
        {
            buffer[writeIndex++] = '0';
        }
    }
    else
    {
        writeIndex = 0;
        while(writeIndex < padSize)
        {
            buffer[writeIndex++] = ' ';
        }
        if(isNegative)
        {
            buffer[writeIndex - 1] = '-';
        }
    }

    return writeIndex;
}

static int IntToStr(char* const buffer, const u32 padSize, const PrintfFlags flags, i32 val)
{
    u32 isNegative = 0;
    u32 unsignedValue = (u32) val;
    u32 writeIndex = 0;

    if(flags.IsSigned && val < 0)
    {
        isNegative = 1;
        if(val == I32_MIN)
        {
            buffer[0] = '-';
            buffer[1] = '2';
            buffer[2] = '1';
            buffer[3] = '4';
            buffer[4] = '7';
            buffer[5] = '4';
            buffer[6] = '8';
            buffer[7] = '3';
            buffer[8] = '6';
            buffer[9] = '4';
            buffer[10] = '8';
            buffer[11] = '\0';
            return 10;
        }

        unsignedValue = (u32) -val;
        buffer[writeIndex++] = '-';
    }
    else if(flags.ForceSign)
    {
        buffer[writeIndex++] = '+';
    }

    if(unsignedValue == 0)
    {
        buffer[writeIndex++] = '0';
        if(padSize != 0)
        {
            if(flags.LeftJustify || flags.PadZeros)
            {
                if(flags.PadZeros)
                {
                    while(writeIndex < padSize)
                    {
                        buffer[writeIndex++] = '0';
                    }
                }
                else
                {
                    while(writeIndex < padSize)
                    {
                        buffer[writeIndex++] = ' ';
                    }
                }
            }
            else
            {
                while(writeIndex < padSize)
                {
                    buffer[writeIndex - 1] = ' ';
                    ++writeIndex;
                }
                buffer[writeIndex++] = '0';
            }
        }
        buffer[writeIndex] = '\0';

        return writeIndex;
    }

    if(!flags.LeftJustify && padSize != 0)
    {
        if(unsignedValue < 10 && padSize > 1)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 1, flags, isNegative);
        }
        else if(unsignedValue < 100 && padSize > 2)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 2, flags, isNegative);
        }
        else if(unsignedValue < 1000 && padSize > 3)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 3, flags, isNegative);
        }
        else if(unsignedValue < 10000 && padSize > 4)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 4, flags, isNegative);
        }
        else if(unsignedValue < 100000 && padSize > 5)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 5, flags, isNegative);
        }
        else if(unsignedValue < 1000000 && padSize > 6)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 6, flags, isNegative);
        }
        else if(unsignedValue < 10000000 && padSize > 7)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 7, flags, isNegative);
        }
        else if(unsignedValue < 100000000 && padSize > 8)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 8, flags, isNegative);
        }
        else if(unsignedValue < 1000000000 && padSize > 9)
        {
            writeIndex = PadIntStr(buffer, writeIndex, padSize - 9, flags, isNegative);
        }
    }

    const u32 numberBegin = writeIndex;

    while(unsignedValue > 0)
    {
        const u32 rem   = unsignedValue / 10;
        const u32 digit = unsignedValue % 10;
        unsignedValue = rem;
        buffer[writeIndex++] = GetDigitCharU(digit);
    }

    const u32 maxFlipIndex = (writeIndex + numberBegin) / 2;

    for(u32 i = numberBegin; i < maxFlipIndex; ++i)
    {
        const u32 index = writeIndex - i;
        const char tmp = buffer[i];
        buffer[i] = buffer[index];
        buffer[index] = tmp;
    }
    buffer[writeIndex] = '\0';

    return writeIndex;
}

FASTCALL_GCC static int FASTCALL_MSVC internal_kvprintf(const char* const format, va_list vList)
{
    int writeCount = 0;

    FormatStage formatStage = FormatStage_None;
    PrintfFlags flags;
    flags.Packed = 0;
    ConsoleColor bg = ConColor_Black;
    ConsoleColor fg = ConColor_White;
    char intBuffer[32];

    char last = '\0';

    for(u32 i = 0; format[i]; ++i)
    {
        const char c = format[i];

        if(formatStage != FormatStage_None)
        {
            if(formatStage == FormatStage_BehaviorFlags)
            {
                switch(c)
                {
                    case '-': // Enable Left Justify
                        flags.LeftJustify = 1;
                        continue; // Continue to prevent flags reset
                    case '+': // Force a sign to always be printed
                        flags.ForceSign = 1;
                        continue; // Continue to prevent flags reset
                    case ' ': // Force a space to printed instead of a sign when positive
                        flags.SpaceSign = 1;
                        continue; // Continue to prevent flags reset
                    case '#': // Alternative form
                        flags.AlternativeForm1 = 1;
                        continue; // Continue to prevent flags reset
                    case '^': // Alternative form 2, non-standard, uses 0x with %X instead of 0X
                        flags.AlternativeForm2 = 1;
                        continue; // Continue to prevent flags reset
                    case '0': // Pad with zeros
                        flags.PadZeros = 1;
                        continue; // Continue to prevent flags reset
                    case '[': // Set Color infomation
                        formatStage = FormatStage_ColorForeground;
                        continue; // Continue to prevent flags reset
                    default:
                        formatStage = FormatStage_FieldWidth;
                        break;
                }
            }
            else if(formatStage == FormatStage_ColorBackground)
            {
                if(c >= '0' && c <= '9')
                {
                    bg = c - '0';
                }
                else if(c >= 'a' && c <= 'f')
                {
                    bg = c - 'a' + 10;
                }
                else if(c >= 'A' && c <= 'F')
                {
                    bg = c - 'A' + 10;
                }
                else if(c == 'r' || c == 'R')
                {
                    bg = ConColor_Black;
                }
                else if(c != '_')
                {
                    return -1;
                }

                formatStage = FormatStage_FieldWidth;
                continue;
            }
            else if(formatStage == FormatStage_ColorForeground)
            {
                if(c >= '0' && c <= '9')
                {
                    fg = c - '0';
                }
                else if(c >= 'a' && c <= 'f')
                {
                    fg = c - 'a' + 10;
                }
                else if(c >= 'A' && c <= 'F')
                {
                    fg = c - 'A' + 10;
                }
                else if(c == 'r' || c == 'R')
                {
                    fg = ConColor_White;
                }
                else if(c != '_')
                {
                    return -1;
                }

                formatStage = FormatStage_ColorBackground;
                continue;
            }

            // No else here because FormatStage_BehaviorFlags can break instead of continue
            if(formatStage == FormatStage_FieldWidth)
            {

            }

            switch(c)
            {
                case '%': // Print a '%' sybmol 
                    ++writeCount;
                    ConWriteCharColor('%', bg, fg);
                    break;
                case 'c': // Print a character
                {
                    const int c = va_arg(vList, int); // char is promoted to int
                    ++writeCount;
                    ConWriteCharColor((char) c, bg, fg);
                    break;
                }
                case 's': // Print a string
                {
                    const char* const str = va_arg(vList, const char*);
                    if(str)
                    {
                        writeCount += ConWriteStringColor(str, bg, fg);
                    }
                    else
                    {
                        writeCount += ConWriteStringColor("null", bg, fg);
                    }
                    break;
                }
                case 'd':
                case 'i': // Print a signed integer.
                {
                    const int i = va_arg(vList, int);
                    flags.IsSigned = 1;
                    IntToStr(intBuffer, 0, flags, i);
                    writeCount += ConWriteStringColor(intBuffer, bg, fg);
                    break;
                }
                case 'u': // Print an unsigned integer.
                {
                    const int i = va_arg(vList, int);
                    flags.IsSigned = 0;
                    IntToStr(intBuffer, 0, flags, i);
                    writeCount += ConWriteStringColor(intBuffer, bg, fg);
                    break;
                }
                case 'x':
                {
                    const u32 i = va_arg(vList, u32);
                    xtoa(i, intBuffer);
                    if(flags.AlternativeForm1)
                    {
                        ConWriteChar('0');
                        ConWriteChar('x');
                        writeCount += 2;
                    }
                    else if(flags.AlternativeForm2)
                    {
                        ConWriteChar('0');
                        ConWriteChar('X');
                        writeCount += 2;
                    }
                    writeCount += ConWriteStringColor(intBuffer, bg, fg);
                    break;
                }
                case 'p':
                {
                    const u32 i = va_arg(vList, u32);
                    Xtoap(i, intBuffer);
                    if(flags.AlternativeForm1)
                    {
                        ConWriteChar('0');
                        ConWriteChar('X');
                        writeCount += 2;
                    }
                    else if(flags.AlternativeForm2)
                    {
                        ConWriteChar('0');
                        ConWriteChar('x');
                        writeCount += 2;
                    }
                    writeCount += ConWriteStringColor(intBuffer, bg, fg);
                    break;
                }
                case 'X':
                {
                    const u32 i = va_arg(vList, u32);
                    Xtoa(i, intBuffer);
                    if(flags.AlternativeForm1)
                    {
                        ConWriteChar('0');
                        ConWriteChar('X');
                        writeCount += 2;
                    }
                    else if(flags.AlternativeForm2)
                    {
                        ConWriteChar('0');
                        ConWriteChar('x');
                        writeCount += 2;
                    }
                    writeCount += ConWriteStringColor(intBuffer, bg, fg);
                    break;
                }
                case ']': break; // Just changing color
            }

            formatStage = FormatStage_None;
            flags.Packed = 0;
        }
        else if(c == '%')
        {
            formatStage = FormatStage_BehaviorFlags;
            continue;
        }
        else
        {
            ++writeCount;

            if(c == '\n')
            {
                // const ConCoord currentPos = ConGetCursorCoord();
                // ConSetCursorCoord(0, currentPos.y + 1);
                ConNewLine();
            }
            else if(c == '\r')
            {
                if(last == '\r')
                {
                    for(u32 i = 0; i < CON_COL_COUNT - 1; ++i)
                    {
                        ConWriteCharColor(' ', ConColor_Black, ConColor_Black);
                    }
                }
                const ConCoord currentPos = ConGetCursorCoord();
                ConSetCursorCoord(0, currentPos.y);
            }
            else if(c == '\v')
            {
                const ConCoord currentPos = ConGetCursorCoord();
                ConSetCursorCoord(currentPos.x, 0);
            }
            else if(c == '\t')
            {
                ConSetCursor((ConGetCursor() + 4) & ~3);
            }
            else if(c == '\b')
            {
                ConSetCursor(ConGetCursor() - 1);
            }
            else 
            {
                ConWriteCharColor(c, bg, fg);
            }
        }
        last = c;
    }
    ConSwapBuffers();

    return writeCount;
}

FASTCALL_GCC static int FASTCALL_MSVC internal_kvsnprintf(char* const buffer, const u32 bufferSize, const char* const format, va_list vList)
{
    (void) buffer;
    (void) bufferSize;
    (void) format;
    (void) vList;

    return 0;
}
