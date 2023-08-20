#pragma once

#ifdef __cplusplus
  #define DECL_ENUM(_NAME) \
    enum _NAME
  #define DECL_ENUM_S(_NAME, _SIZE_TYPE) \
    enum _NAME : _SIZE_TYPE
#else
  #define DECL_ENUM(_NAME) \
    typedef enum _NAME _NAME; \
    enum _NAME
  #define DECL_ENUM_S(_NAME, _SIZE_TYPE) \
    typedef enum _NAME _NAME; \
    enum _NAME
#endif

#define ENUM_FORCE_U8  _FORCE_U8  = 0x7F
#define ENUM_FORCE_U16 _FORCE_U16 = 0x7FFF
#define ENUM_FORCE_U32 _FORCE_U32 = 0x7FFFFFFF
