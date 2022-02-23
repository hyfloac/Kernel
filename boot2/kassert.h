#pragma once

#ifdef __cplusplus
  #define STATIC_ASSERT static_assert
#else
  #define STATIC_ASSERT _Static_assert
#endif
