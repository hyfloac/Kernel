#pragma once

#define DECL_ENUM(NAME) \
  typedef enum NAME

#if defined(__cplusplus) || defined(USE_SIZED_ENUM)
  #define DECL_ENUM_S(NAME, SIZE_TYPE) \
    typedef enum NAME : SIZE_TYPE
#else
  #define DECL_ENUM_S(NAME, SIZE_TYPE) \
    typedef enum NAME
#endif

#define ENUM_FORCE_U8  _FORCE_U8  = 0x7F
#define ENUM_FORCE_U16 _FORCE_U16 = 0x7FFF
#define ENUM_FORCE_U32 _FORCE_U32 = 0x7FFFFFFF
