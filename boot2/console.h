#pragma once

#include "callspec.h"
#include "kstdint.h"
#include "kassert.h"
#include "enum.h"

#define CON_ROW_COUNT (25)
#define CON_COL_COUNT (80)
#define CON_MAX_CHAR (CON_ROW_COUNT * CON_COL_COUNT)
#define CON_VGA_MEM (0xB8000)
#define CON_BACK_BUFFER (0x4F060)

#ifdef __cplusplus
extern "C" {
#endif

DECL_ENUM_S(ConsoleColor, u8)
{
    ConColor_Black          = 0x0,
    ConColor_Blue           = 0x1,
    ConColor_Green          = 0x2,
    ConColor_Cyan           = 0x3,
    ConColor_Red            = 0x4,
    ConColor_Magenta        = 0x5,
    ConColor_Brown          = 0x6,
    ConColor_LightGray      = 0x7,
    ConColor_DarkGray       = 0x8,
    ConColor_LightBlue      = 0x9,
    ConColor_LightGreen     = 0xA,
    ConColor_LightCyan      = 0xB,
    ConColor_LightRed       = 0xC,
    ConColor_LightMagenta   = 0xD,
    ConColor_Yellow         = 0xE,
    ConColor_White          = 0xF
} ConsoleColor;

typedef struct
{
    char c;
    union
    {
        struct
        {
            ConsoleColor fg : 4;
            ConsoleColor bg : 4;
        } __attribute__((packed));
        u8 color;
    } __attribute__((packed));
} __attribute__((packed)) Character;

STATIC_ASSERT(sizeof(Character) == 2, "Character was not 2 bytes in size.");

typedef struct ConCoord
{
    u32 x;
    u32 y;
} ConCoord;

/**
 * @brief Initialize the console.
 * 
 * This will clear the back buffer and swap it to the front buffer. 
 */
void ConInit();

/**
 * @brief "Swaps" the back buffer to the front buffer.
 * 
 *   Buffers are not swapped due to the VGA memory being pinned, 
 * instead data is copied from the back buffer to the front buffer.
 */
void ConSwapBuffers();

/**
 * @brief Clears the screen with spaces.
 * 
 */
void ConClearScreen();

/**
 * @brief Move the cursor to the beginning of a new line.
 * 
 * If this overflows the console the buffer will be shifted up by one row.
 */
void ConNewLine();

FASTCALL_GCC void FASTCALL_MSVC ConSetCursor(u32 index);
FASTCALL_GCC void FASTCALL_MSVC ConSetCursorCoord(u32 x, u32 y);
u32 ConGetCursor();
ConCoord ConGetCursorCoord();
 
FASTCALL_GCC void FASTCALL_MSVC ConWriteChar(char c);
FASTCALL_GCC void FASTCALL_MSVC ConWriteCharacater(Character c);
FASTCALL_GCC void FASTCALL_MSVC ConWriteCharColor(char c, ConsoleColor bg, ConsoleColor fg);
 
FASTCALL_GCC u32 FASTCALL_MSVC ConWriteString(const char* str);
FASTCALL_GCC u32 FASTCALL_MSVC ConWriteCharacterString(const Character* str);
FASTCALL_GCC u32 FASTCALL_MSVC ConWriteStringColor(const char* str, ConsoleColor bg, ConsoleColor fg);

FASTCALL_GCC void FASTCALL_MSVC ConBackspace(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
#include "enum_operator.hpp"

ENUM_OP(ConsoleColor, u8)

namespace Console {

inline void Init() noexcept { ConInit(); }
inline void SwapBuffers() noexcept { ConSwapBuffers(); }
inline void ClearScreen() noexcept { ConClearScreen(); }

FASTCALL_GCC inline void FASTCALL_MSVC SetCursor(const u32 index) noexcept { ConSetCursor(index); }
FASTCALL_GCC inline void FASTCALL_MSVC SetCursor(const u32 x, const u32 y) noexcept { ConSetCursorCoord(x, y); }
u32 GetCursor() noexcept { return ConGetCursor(); }
ConCoord GetCursorCoord() noexcept { return ConGetCursorCoord(); }
 
FASTCALL_GCC inline void FASTCALL_MSVC WriteChar(const char c) noexcept { ConWriteChar(c); }
FASTCALL_GCC inline void FASTCALL_MSVC WriteChar(const Character c) noexcept { ConWriteCharacater(c); }
FASTCALL_GCC inline void FASTCALL_MSVC WriteChar(const char c, const ConsoleColor bg, const ConsoleColor fg) noexcept { ConWriteCharColor(c, bg, fg); }

FASTCALL_GCC inline u32 FASTCALL_MSVC WriteString(const char* const str) noexcept { return ConWriteString(str); }
FASTCALL_GCC inline u32 FASTCALL_MSVC WriteString(const Character* const str) noexcept { return ConWriteCharacterString(str); }
FASTCALL_GCC inline u32 FASTCALL_MSVC WriteString(const char* const str, const ConsoleColor bg, const ConsoleColor fg) noexcept { return ConWriteStringColor(str, bg, fg); }

FASTCALL_GCC inline void FASTCALL_MSVC Backspace() noexcept { ConBackspace(); }

}
#endif
