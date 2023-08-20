#pragma once

#include "kerror.h"

#ifdef __cplusplus
extern "C" {
#endif

KError_t InitCommandLine(void);

typedef i32(* CommandLineCommand_f)(const char* const name, const u32 argCount, const char* args[]);

void CommandLineRegisterCommand(const char* const name, const CommandLineCommand_f commandFunc);

#ifdef __cplusplus
} /* extern "C" */
#endif
