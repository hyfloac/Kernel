#pragma once

#include "kstdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitKAlloc(void);

void* kalloc(uSys size);
void kfree(void* data);

#ifdef __cplusplus
} /* extern "C" */
#endif
