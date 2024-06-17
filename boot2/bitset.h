#pragma once

#include "kstdint.h"
#include "callspec.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC static inline u32 FASTCALL_MSVC BitSet_Words(const u32 bitCount)
{
    const u32 minWords = bitCount / (sizeof(u32) * CHAR_BIT);
    const u32 remBits = bitCount % (sizeof(u32) * CHAR_BIT);

    return minWords + (remBits != 0 ? 1 : 0);
}

FASTCALL_GCC static inline u32 FASTCALL_MSVC BitSet_Bits(const u32 wordCount)
{
    return wordCount * (sizeof(u32) * CHAR_BIT);
}

FASTCALL_GCC static inline void FASTCALL_MSVC BitSet_SetBit(u32* const words, const u32 bit)
{
    const u32 wordIndex = bit / sizeof(u32);
    const u32 bitIndex = bit % sizeof(u32);
    const u32 invBitIndex = (sizeof(u32) * CHAR_BIT) - bitIndex;

    words[wordIndex] |= (1 << invBitIndex);
}

FASTCALL_GCC static inline void FASTCALL_MSVC BitSet_UnsetBit(u32* const words, const u32 bit)
{
    const u32 wordIndex = bit / sizeof(u32);
    const u32 bitIndex = bit % sizeof(u32);
    const u32 invBitIndex = (sizeof(u32) * CHAR_BIT) - bitIndex;

    words[wordIndex] &= ~(1 << invBitIndex);
}

FASTCALL_GCC static inline void FASTCALL_MSVC BitSet_SetBitBool(u32* const words, const u32 bit, const u32 enabled)
{
    if(enabled)
    {
        BitSet_SetBit(words, bit);
    }
    else
    {
        BitSet_UnsetBit(words, bit);
    }
}

FASTCALL_GCC static inline u32 FASTCALL_MSVC BitSet_FlipBit(u32* const words, const u32 bit)
{
    const u32 wordIndex = bit / sizeof(u32);
    const u32 bitIndex = bit % sizeof(u32);
    const u32 invBitIndex = (sizeof(u32) * CHAR_BIT) - bitIndex;
    const u32 bitMask = 1 << invBitIndex;

    words[wordIndex] ^= bitMask;

    return words[wordIndex] & bitMask;
}

FASTCALL_GCC static inline u32 FASTCALL_MSVC BitSet_GetBit(const u32* const words, const u32 bit)
{
    const u32 wordIndex = bit / sizeof(u32);
    const u32 bitIndex = bit % sizeof(u32);
    const u32 invBitIndex = (sizeof(u32) * CHAR_BIT) - bitIndex;

    return words[wordIndex] & (1 << invBitIndex);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus

class BitSetWrapper final
{
private:
    u32 _bitCount;
    u32* _words;
public:
    BitSetWrapper(const u32 bitCount, u32* const words) noexcept
        : _bitCount(bitCount)
        , _words(words)
    { }

    [[nodiscard]] u32  BitCount() const noexcept { return _bitCount; }
    [[nodiscard]] u32 WordCount() const noexcept { return _bitCount / (sizeof(u32) * CHAR_BIT); }
    [[nodiscard]]       u32* Words()       noexcept { return _words; }
    [[nodiscard]] const u32* Words() const noexcept { return _words; }

    [[nodiscard]] bool operator[](const u32 bit) const noexcept
    {
        return BitSet_GetBit(_words, bit);
    }
    
    [[nodiscard]] bool GetBit(const u32 bit) noexcept
    {
        return BitSet_GetBit(_words, bit);
    }
    
    void SetBit(const u32 bit) noexcept
    {
        BitSet_SetBit(_words, bit);
    }
    
    void UnsetBit(const u32 bit) noexcept
    {
        BitSet_UnsetBit(_words, bit);
    }
    
    void SetBit(const u32 bit, const bool value) noexcept
    {
        BitSet_SetBitBool(_words, bit, value);
    }

    bool FlipBit(const u32 bit) noexcept
    {
        return BitSet_FlipBit(_words, bit);
    }
};

#endif
