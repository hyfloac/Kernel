#include "kstring.h"

FASTCALL_GCC u32 FASTCALL_MSVC StrLen(const char* const str)
{
    u32 len = 0;
    for(; str[len]; ++len);
    return len;
}
