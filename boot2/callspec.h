#pragma once

#if defined(__i386__)
  #ifdef _MSVC_LANG
    #define FASTCALL_MSVC __fastcall
    #define FASTCALL_GCC

    #define STDCALL_MSVC __stdcall
    #define STDCALL_GCC

    #define CDECL_MSVC __cdecl
    #define CDECL_GCC

    #define VECTORCALL_MSVC __vectorcall
    #define VECTORCALL_GCC
  #else
    #define FASTCALL_MSVC
    #define FASTCALL_GCC __attribute__((fastcall))

    #define STDCALL_MSVC
    #define STDCALL_GCC __attribute__((stdcall))

    #define CDECL_MSVC
    #define CDECL_GCC __attribute__((cdecl))

    #define VECTORCALL_MSVC
    #define VECTORCALL_GCC __attribute__((vectorcall))
  #endif
#elif defined(__x86_64__)
  #define FASTCALL_MSVC
  #define FASTCALL_GCC

  #define STDCALL_MSVC
  #define STDCALL_GCC

  #define CDECL_MSVC
  #define CDECL_GCC

  #define VECTORCALL_MSVC
  #define VECTORCALL_GCC
#else
  #error Unsupported Architecture
#endif
