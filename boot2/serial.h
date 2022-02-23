#pragma once

#include "kstdint.h"
#include "callspec.h"
#include "kerror.h"

#ifdef __cplusplus
extern "C" {
#endif

FASTCALL_GCC KError_t FASTCALL_MSVC SerialInit(void* readBuffer, uSys readBufferSize, void* writeBuffer, uSys writeBufferSize);

FASTCALL_GCC void FASTCALL_MSVC SerialReInitBuffers(void* readBuffer, uSys readBufferSize, void* writeBuffer, uSys writeBufferSize);

void SerialUpdateTick();

int SerialIsReadReady();
int SerialIsWriteReady();

char SerialReadCharBlocking();
FASTCALL_GCC int SerialReadCharNonBlocking(char* c);

FASTCALL_GCC int FASTCALL_MSVC SerialWriteCharBlocking(char c);
FASTCALL_GCC int FASTCALL_MSVC SerialWriteCharNonBlocking(char c);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus

namespace Serial {

bool Init(void* const readBuffer, const uSys readBufferSize, void* const writeBuffer, const uSys writeBufferSize) noexcept { return SerialInit(readBuffer, readBufferSize, writeBuffer, writeBufferSize); }

[[nodiscard]] bool IsDataReady() noexcept { return SerialIsReadReady(); }
[[nodiscard]] bool IsWriteReady() noexcept { return SerialIsWriteReady(); }

char ReadCharBlocking() noexcept { return SerialReadCharBlocking(); }
bool ReadCharNonBlocking(char* const c) noexcept { return SerialReadCharNonBlocking(c); }

bool WriteCharBlocking(const char c) noexcept { return SerialWriteCharBlocking(c); }
bool WriteCharNonBlocking(const char c) noexcept { return SerialWriteCharNonBlocking(c); }

}

#endif
