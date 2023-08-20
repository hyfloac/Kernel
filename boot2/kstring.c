#include "kstring.h"
#include "kmaths.h"

FASTCALL_GCC u32 FASTCALL_MSVC StrLen(const char* const str)
{
    u32 len = 0;
    for(; str[len]; ++len);
    return len;
}

FASTCALL_GCC i32 FASTCALL_MSVC StrCmpLen(const char* const str0, const uSys str0Length, const char* const str1, const uSys str1Length)
{
    if(!str0)
    {
        return -1;
    }

    if(!str1)
    {
        return 1;
    }

    if(str0 == str1)
    {
        if(str0Length == str1Length)
        {
            return 0;
        }
        else if(str0Length < str1Length)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }

    if(str0Length != str1Length)
    {
        if(str0Length == 0)
        {
            return 1;
        }
        else if(str1Length == 0)
        {
            return -1;
        }
    }

    uSys maxLength = KM_MIN(str0Length, str1Length);

    for(uSys i = 0; i < maxLength; ++i)
    {
        if(str0[i] == str1[i])
        {
            continue;
        }
        if(str0[i] < str1[i])
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}
