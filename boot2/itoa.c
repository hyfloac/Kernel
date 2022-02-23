#include "itoa.h"

FASTCALL_GCC static char FASTCALL_MSVC GetCharPointU(u32 x);
FASTCALL_GCC static char FASTCALL_MSVC GetCharPointL(u32 x);
FASTCALL_GCC static void FASTCALL_MSVC ReverseArr(char* arr, u32 len);

FASTCALL_GCC void FASTCALL_MSVC btoa(u32 x, char str[33])
{
    if(x == 0)
    {
        str[0] = '0';
        str[1] = '\0';
    }
    else
    {
        u32 index = 0;
        {
            while(1)
            {
                const u32 bit = x & 0x80000000;
                if(bit == 1)
                {
                    break;
                }
                x <<= 1;
                ++index;
            };
        }
        
        do
        {
            const u32 bit = x & 0x80000000;
            x <<= 1;
            str[index++] = bit == 0 ? '0' : '1';
        } while (index < 32);
        str[32] = '\0';
    }
}

FASTCALL_GCC void FASTCALL_MSVC btoap(u32 x, char str[33])
{
    u32 index = 0;
    
    do
    {
        const u32 bit = x & 0x80000000;
        x <<= 1;
        str[index++] = bit == 0 ? '0' : '1';
    } while (index < 32);
    str[32] = '\0';
}

FASTCALL_GCC void FASTCALL_MSVC xtoa(u32 x, char str[9])
{
    if(x != 0)
    {
        u32 i = 0;
        do
        {
            const u32 digit = x % 16;
            x /= 16;
            str[i++] = GetCharPointL(digit);
        } while(x != 0); 
        str[i] = '\0';
        ReverseArr(str, i);
    }
    else
    {
        str[0] = '0';
        str[1] = '\0';
    }
}

FASTCALL_GCC void FASTCALL_MSVC Xtoa(u32 x, char str[9])
{
    if(x != 0)
    {
        u32 i = 0;
        do
        {
            const u32 digit = x % 16;
            x /= 16;
            str[i++] = GetCharPointU(digit);
        } while(x != 0);   
        str[i] = '\0';
        ReverseArr(str, i);
    }
    else
    {
        str[0] = '0';
        str[1] = '0';
        str[2] = '0';
        str[3] = '0';
        str[4] = '0';
        str[5] = '0';
        str[6] = '0';
        str[7] = '0';
        str[8] = '\0';
    }
}

FASTCALL_GCC void FASTCALL_MSVC xtoap(u32 x, char str[9])
{
    if(x != 0)
    {
        u32 i = 0;
        do
        {
            const u32 digit = x % 16;
            x /= 16;
            str[i++] = GetCharPointL(digit);
        } while(x != 0);

        for(; i < 8; ++i)
        {
            str[i] = '0';
        }

        str[i] = '\0';
        ReverseArr(str, 8);
    }
    else
    {
        str[0] = '0';
        str[1] = '0';
        str[2] = '0';
        str[3] = '0';
        str[4] = '0';
        str[5] = '0';
        str[6] = '0';
        str[7] = '0';
        str[8] = '\0';
    }
}

FASTCALL_GCC void FASTCALL_MSVC Xtoap(u32 x, char str[9])
{
    if(x != 0)
    {
        u32 i = 0;
        do
        {
            const u32 digit = x % 16;
            x /= 16;
            str[i++] = GetCharPointU(digit);
        } while(x != 0);

        for(; i < 8; ++i)
        {
            str[i] = '0';
        }

        str[i] = '\0';
        ReverseArr(str, 8);
    }
    else
    {
        str[0] = '0';
        str[1] = '0';
        str[2] = '0';
        str[3] = '0';
        str[4] = '0';
        str[5] = '0';
        str[6] = '0';
        str[7] = '0';
        str[8] = '\0';
    }
}

FASTCALL_GCC void FASTCALL_MSVC utoa(u32 x, char str[11])
{
    if(x != 0)
    {
        u32 i = 0;
        do
        {
            const u32 digit = x % 10;
            x /= 10;
            str[i++] = GetCharPointL(digit);
        } while(x != 0);

        str[i] = '\0';
        ReverseArr(str, 8);
    }
    else
    {
        str[0] = '0';
        str[8] = '\0';
    }
}

FASTCALL_GCC void FASTCALL_MSVC itoa(i32 x, char str[12])
{
    if(x < 0)
    {
        str[0] = '-';
        if((u32) x != 0xFFFFFFFF)
        {
            utoa((u32) -x, str + 1);
        }
        else
        {
            str[1] = '2';
            str[2] = '1';
            str[3] = '4';
            str[4] = '7';
            str[5] = '4';
            str[6] = '8';
            str[7] = '3';
            str[8] = '6';
            str[9] = '4';
            str[10] = '8';
            str[11] = '\0';
        }
    }
    else
    {
        utoa((u32) x, str);
    }
}

FASTCALL_GCC static void FASTCALL_MSVC ReverseArr(char* const arr, const u32 len)
{
    for(u32 i = 0, mid = len / 2; i < mid; ++i)
    {
        const u32 index = len - i - 1;
        const char tmp = arr[i];
        arr[i] = arr[index];
        arr[index] = tmp;
    }
}

FASTCALL_GCC static char FASTCALL_MSVC GetCharPointU(const u32 x)
{
    switch(x)
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
        case 0xA: return 'A';
        case 0xB: return 'B';
        case 0xC: return 'C';
        case 0xD: return 'D';
        case 0xE: return 'E';
        case 0xF: return 'F';
        default:  return '#';
    }
}

FASTCALL_GCC static char FASTCALL_MSVC GetCharPointL(const u32 x)
{
    switch(x)
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
        case 0xA: return 'a';
        case 0xB: return 'b';
        case 0xC: return 'c';
        case 0xD: return 'd';
        case 0xE: return 'e';
        case 0xF: return 'f';
        default:  return '#';
    }
}
