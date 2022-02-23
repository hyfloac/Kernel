#pragma once

#ifdef __cplusplus
  #define ENUM_OP_OR(_ENUM, _SIZE_T) \
    static inline _ENUM operator |(const _ENUM a, const _ENUM b)                \
    { const _SIZE_T res = static_cast<_SIZE_T>(a) | static_cast<_SIZE_T>(b);    \
      return static_cast<_ENUM>(res); }                                         \
    static inline _ENUM operator |(const _SIZE_T a, const _ENUM b)              \
    { const _SIZE_T res = a | static_cast<_SIZE_T>(b);                          \
      return static_cast<_ENUM>(res); }                                         \
    static inline _ENUM operator |(const _ENUM a, const _SIZE_T b)              \
    { const _SIZE_T res = static_cast<_SIZE_T>(a) | b;                          \
      return static_cast<_ENUM>(res); }

  #define ENUM_OP_AND(_ENUM, _SIZE_T) \
    static inline _ENUM operator &(const _ENUM a, const _ENUM b)                \
    { const _SIZE_T res = static_cast<_SIZE_T>(a) & static_cast<_SIZE_T>(b);    \
      return static_cast<_ENUM>(res); }                                         \
    static inline _ENUM operator &(const _SIZE_T a, const _ENUM b)              \
    { const _SIZE_T res = a & static_cast<_SIZE_T>(b);                          \
      return static_cast<_ENUM>(res); }                                         \
    static inline _ENUM operator &(const _ENUM a, const _SIZE_T b)              \
    { const _SIZE_T res = static_cast<_SIZE_T>(a) & b;                          \
      return static_cast<_ENUM>(res); }

  #define ENUM_OP_XOR(_ENUM, _SIZE_T) \
    static inline _ENUM operator ^(const _ENUM a, const _ENUM b)                \
    { const _SIZE_T res = static_cast<_SIZE_T>(a) ^ static_cast<_SIZE_T>(b);    \
      return static_cast<_ENUM>(res); }                                         \
    static inline _ENUM operator ^(const _SIZE_T a, const _ENUM b)              \
    { const _SIZE_T res = a ^ static_cast<_SIZE_T>(b);                          \
      return static_cast<_ENUM>(res); }                                         \
    static inline _ENUM operator ^(const _ENUM a, const _SIZE_T b)              \
    { const _SIZE_T res = static_cast<_SIZE_T>(a) ^ b;                          \
      return static_cast<_ENUM>(res); }

  #define ENUM_OP_COR(_ENUM, _SIZE_T) \
    static inline _ENUM& operator |=(_ENUM& a, const _ENUM b)                   \
    { const _ENUM res = a | b;                                                  \
      a = res;                                                                  \
      return a; }                                                               \
    static inline _SIZE_T& operator |=(_SIZE_T& a, const _ENUM b)               \
    { const _SIZE_T res = a | static_cast<_SIZE_T>(b);                          \
      a = res;                                                                  \
      return a; }                                                               \
    static inline _ENUM& operator |=(_ENUM& a, const _SIZE_T b)                 \
    { const _ENUM res = a | b;                                                  \
      a = res;                                                                  \
      return a; } 

  #define ENUM_OP_CAND(_ENUM, _SIZE_T) \
    static inline _ENUM& operator &=(_ENUM& a, const _ENUM b)                   \
    { const _ENUM res = a & b;                                                  \
      a = res;                                                                  \
      return a; }                                                               \
    static inline _SIZE_T& operator &=(_SIZE_T& a, const _ENUM b)               \
    { const _SIZE_T res = a & static_cast<_SIZE_T>(b);                          \
      a = res;                                                                  \
      return a; }                                                               \
    static inline _ENUM& operator &=(_ENUM& a, const _SIZE_T b)                 \
    { const _ENUM res = a & b;                                                  \
      a = res;                                                                  \
      return a; } 

  #define ENUM_OP_CXOR(_ENUM, _SIZE_T) \
    static inline _ENUM& operator ^=(_ENUM& a, const _ENUM b)                   \
    { const _ENUM res = a ^ b;                                                  \
      a = res;                                                                  \
      return a; }                                                               \
    static inline _SIZE_T& operator ^=(_SIZE_T& a, const _ENUM b)               \
    { const _SIZE_T res = a ^ static_cast<_SIZE_T>(b);                          \
      a = res;                                                                  \
      return a; }                                                               \
    static inline _ENUM& operator ^=(_ENUM& a, const _SIZE_T b)                 \
    { const _ENUM res = a ^ b;                                                  \
      a = res;                                                                  \
      return a; } 
#else
  #define ENUM_OP_OR(_ENUM, _SIZE_T)
  #define ENUM_OP_AND(_ENUM, _SIZE_T)
  #define ENUM_OP_XOR(_ENUM, _SIZE_T)
  #define ENUM_OP_COR(_ENUM, _SIZE_T)
  #define ENUM_OP_CAND(_ENUM, _SIZE_T)
  #define ENUM_OP_CXOR(_ENUM, _SIZE_T)
#endif

#define ENUM_OP(_ENUM, _SIZE_T)  \
    ENUM_OP_OR(_ENUM, _SIZE_T)   \
    ENUM_OP_AND(_ENUM, _SIZE_T)  \
    ENUM_OP_XOR(_ENUM, _SIZE_T)  \
    ENUM_OP_COR(_ENUM, _SIZE_T)  \
    ENUM_OP_CAND(_ENUM, _SIZE_T) \
    ENUM_OP_CXOR(_ENUM, _SIZE_T)
