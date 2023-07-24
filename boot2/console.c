#include "console.h"
#include "kassert.h"

static Character* ConMem = (Character*) CON_VGA_MEM;
static Character* ConBackBuffer = (Character*) CON_BACK_BUFFER;
static u32 ConIndex = 0;

FASTCALL_GCC static u32 FASTCALL_MSVC CoordToIndex(const u32 x, const u32 y);
FASTCALL_GCC static ConCoord FASTCALL_MSVC IndexToCoord(const u32 index);
static void MemoryCopyBuffers();
static void ClearBackBuffer();
static void PhaseUpOne();

STATIC_ASSERT(sizeof(Character) == sizeof(u16), "Character is not a u16");

void ConInit()
{
    ConClearScreen();
    ConSwapBuffers();
}

void ConSwapBuffers()
{
    MemoryCopyBuffers();
}

void ConClearScreen()
{
    ClearBackBuffer();
}

void ConNewLine()
{
    ConCoord coord = IndexToCoord(ConIndex);
    coord.x = 0;
    ++coord.y;
    if(coord.y == CON_ROW_COUNT)
    {
        --coord.y;
        PhaseUpOne();
    }
    ConSetCursorCoord(coord.x, coord.y);
}

FASTCALL_GCC void FASTCALL_MSVC ConSetCursor(const u32 index)
{
    ConIndex = index;
}

FASTCALL_GCC void FASTCALL_MSVC ConSetCursorCoord(const u32 x, const u32 y)
{
    ConIndex = CoordToIndex(x, y);
}

u32 ConGetCursor()
{
    return ConIndex;
}

ConCoord ConGetCursorCoord()
{
    return IndexToCoord(ConIndex);
}

static void CheckNewLine()
{
    if(ConIndex >= CON_MAX_CHAR)
    {
        ConIndex = (CON_ROW_COUNT - 1) * CON_COL_COUNT;
        PhaseUpOne();
    }
}

FASTCALL_GCC void FASTCALL_MSVC ConWriteChar(const char c)
{
    ConBackBuffer[ConIndex].fg = ConColor_White;
    ConBackBuffer[ConIndex].bg = ConColor_Black;
    ConBackBuffer[ConIndex].c = c;
    ++ConIndex;
    CheckNewLine();
}

FASTCALL_GCC void FASTCALL_MSVC ConWriteCharacater(const Character c)
{
    ConBackBuffer[ConIndex++] = c;
    CheckNewLine();
}

FASTCALL_GCC void FASTCALL_MSVC ConWriteCharColor(const char c, const ConsoleColor bg, const ConsoleColor fg)
{
    ConBackBuffer[ConIndex].fg = fg;
    ConBackBuffer[ConIndex].bg = bg;
    ConBackBuffer[ConIndex].c = c;
    ++ConIndex;
    CheckNewLine();
}

FASTCALL_GCC u32 FASTCALL_MSVC ConWriteString(const char* const str)
{
    u32 i = 0;
    for(; str[i]; ++i)
    {
        ConBackBuffer[ConIndex].fg = ConColor_White;
        ConBackBuffer[ConIndex].bg = ConColor_Black;
        ConBackBuffer[ConIndex].c = str[i];
        ++ConIndex;
        CheckNewLine();
    }
    return i;
}

FASTCALL_GCC u32 FASTCALL_MSVC ConWriteCharacterString(const Character* const str)
{
    u32 i = 0;
    for(; str[i].c; ++i)
    {
        ConBackBuffer[ConIndex++] = str[i];
        CheckNewLine();
    }
    return i;
}

FASTCALL_GCC u32 FASTCALL_MSVC ConWriteStringColor(const char* const str, const ConsoleColor bg, const ConsoleColor fg)
{
    u32 i = 0;
    for(; str[i]; ++i)
    {
        ConBackBuffer[ConIndex].bg = bg;
        ConBackBuffer[ConIndex].fg = fg;
        ConBackBuffer[ConIndex].c = str[i];
        ++ConIndex;
        CheckNewLine();
    }
    return i;
}

FASTCALL_GCC void FASTCALL_MSVC ConBackspace(void)
{
    const u32 cursor = ConGetCursor() - 1;
    ConSetCursor(cursor);
    ConWriteCharColor(' ', ConColor_Black, ConColor_Black);
    ConSetCursor(cursor);
}

FASTCALL_GCC static u32 FASTCALL_MSVC CoordToIndex(const u32 x, const u32 y)
{
    return CON_COL_COUNT * y + x;
}

FASTCALL_GCC static ConCoord FASTCALL_MSVC IndexToCoord(const u32 index)
{
    ConCoord coord;
    coord.x = index % CON_COL_COUNT;
    coord.y = index / CON_COL_COUNT;
    return coord;
}

#pragma GCC diagnostic push
// We know that the characters are 2 bytes, start at the 32,768 byte boundary, and that the rows are divisible by 16 (2 chars, 8 unrolled times).
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

static void MemoryCopyBuffers()
{
    const u32 byteCount = CON_ROW_COUNT * CON_COL_COUNT * sizeof(Character);
    const u32 wordCount = byteCount / sizeof(u32);

    u32* const conBuffer = (u32*) ConMem;
    const u32* const conBackBuffer = (const u32*) ConBackBuffer;

    for(u32 i = 0; i < wordCount;)
    {
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
        conBuffer[i] = conBackBuffer[i]; i++;
    }
}

static void ClearBackBuffer()
{
    const u32 byteCount = CON_ROW_COUNT * CON_COL_COUNT * 2;
    const u32 wordCount = byteCount / sizeof(u32);

    u32* const conBackBuffer = (u32*) ConBackBuffer;

    const u16 writeChar = (u16) ' '; // Black Space
    const u32 writeWord = (writeChar << 16) | writeChar;

    for(u32 i = 0; i < wordCount;)
    {
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
    }
}

static void PhaseUpOne()
{
    const u32 byteCount = CON_ROW_COUNT * CON_COL_COUNT * 2;
    const u32 wordCount = byteCount / sizeof(u32);

    u32* const conBackBuffer = (u32*) ConBackBuffer;

    const u16 writeChar = (u16) ' '; // Black Space
    const u32 writeWord = (writeChar << 16) | writeChar;

    const u32 copyRowOffset = (CON_COL_COUNT * 2) / sizeof(u32);
    const u32 clearRowOffset = copyRowOffset * (CON_ROW_COUNT - 1);

    for(u32 i = copyRowOffset; i < wordCount;)
    {
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
        conBackBuffer[i - copyRowOffset] = conBackBuffer[i]; ++i;
    }

    for(u32 i = clearRowOffset; i < wordCount;)
    {
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
        conBackBuffer[i++] = writeWord;
    }
}

#pragma GCC diagnostic pop
