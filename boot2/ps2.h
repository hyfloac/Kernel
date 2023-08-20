#pragma once

#include "kstdint.h"
#include "keyboard.h"

#ifdef __cplusplus
extern "C" {
#endif

int PS2Init(void);

u8 PS2ReadData(void);

u32 PS2ReadKeyCode(void);
b8 PS2IsKeyPressed(const u32 keyCode);
KeyboardToggleBits PS2GetKeyboardToggleBits(void);

struct PhysicalDeviceObject;

KError_t RegisterPS2KeyboardDriver(void);
PhysicalDeviceObject* GetPS2KeyboardPDO(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
