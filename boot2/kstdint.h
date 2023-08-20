#pragma once

// #include "stdint.h"

typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed long long i64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef u8 b8;

#ifdef KERNEL_LONG_MODE
typedef i64 iSys;
typedef u64 uSys;
#else
typedef i32 iSys;
typedef u32 uSys;
#endif

#ifdef __INTPTR_TYPE__
typedef __INTPTR_TYPE__ iPtr;
#else
  #ifdef KERNEL_LONG_MODE
typedef i64 iPtr;
  #else
typedef i32 iPtr;
  #endif
#endif

#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ uPtr;
#else
  #ifdef KERNEL_LONG_MODE
typedef u64 uPtr;
  #else
typedef u32 uPtr;
  #endif
#endif

#ifndef CHAR_BIT
  #define CHAR_BIT (8)
#endif

#ifndef NULL
  // #define NULL ((void*) 0)
  #define NULL (0)
#endif

#ifndef FALSE
  #define FALSE (0) 
#endif

#ifndef TRUE
  #define TRUE (1)
#endif

#ifndef __cplusplus
  #define nullptr NULL
  #define false FALSE
  #define true TRUE
#endif


#define I8_MAX  ((i8)  0x7F)
#define I16_MAX ((i16) 0x7FFF)
#define I32_MAX ((i32) 0x7FFFFFFF)
#define I64_MAX ((i64) 0x7FFFFFFFFFFFFFFFull)

#define I8_MIN  ((i8)  0x80)
#define I16_MIN ((i16) 0x8000)
#define I32_MIN ((i32) 0x80000000)
#define I64_MIN ((i64) 0x8000000000000000ull)

#define U8_MAX  ((u8)  0xFF)
#define U16_MAX ((u16) 0xFFFF)
#define U32_MAX ((u32) 0xFFFFFFFF)
#define U64_MAX ((u32) 0xFFFFFFFFFFFFFFFFull)
