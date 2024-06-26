#pragma once

#include "kstdint.h"
#include "callspec.h"

/** 
 * Errors less than 0 are generic kernel errors
 *   Errors greater than zero, that don't have the high bit set, are
 * local errors. The upper 16 bits define the local namespace, the high
 * bit is always 0, leaving only 15 bits to the namespace.
 */
typedef i32 KError_t;

#define KE_IS_OK(Error) (((i32) (Error)) >= 0)

#define KE_OK (0)

#define KE_FAIL (-1)
#define KE_INVALID_ARG (-2)
#define KE_OUT_OF_MEMORY (-3)

#define KE_PAGING_NO_VALID_PAGES (-64)
#define KE_PAGING_NO_32_BIT_PAGES (-65)
#define KE_PAGING_NO_MORE_PLL (-66)

#define KE_DRIVER_TYPE_MISMATCH (-70)
#define KE_DRIVER_SYSTEM_TOO_OLD (-71)
#define KE_DRIVER_TYPE_NOT_FOUND (-72)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Localize an error code into a string.
 * 
 * @param error The error code to localize.
 * @return The localized error code.
 */
FASTCALL_GCC const char* FASTCALL_MSVC KernelLocalizeErrorCode(KError_t error);

/**
 * @brief Register a namespaced error localizer.
 * 
 * @param errorNamespace The 15 bit namespace for which the error code resides
 * @param callback A funciton which will localize the error code. The error passed in will include the space
 */
FASTCALL_GCC void FASTCALL_MSVC KernelRegisterErrorLocalizer(i16 errorNamespace, const char*(FASTCALL_GCC * callback FASTCALL_MSVC)(KError_t));

/**
 * @brief Set a detailed error message
 * 
 * @param message The error message.
 * @param freeOnChange Set to zero for a static string, set to non zero
 *      to have kfree invoked when a new message is set.
 */
FASTCALL_GCC void FASTCALL_MSVC KernelSetErrorMessage(const char* message, const bool freeOnChange);

/**
 * @brief Clears the currently set error message.
 */
static inline void KernelClearErrorMessage()
{
    KernelSetErrorMessage(NULL, 0);
}

/**
 * @brief Get a detailed error message. 
 * 
 * If the error has already been acquired this will return null.
 * 
 * @param acquire If the message was dynamically allocated the caller
 *      will now take responsibility for freeing it. If the error 
 *      cannot be acquired, either because it was statically allocated,
 *      or another user to acquired it, this will return null.
 * @return The detailed error message.
 */
FASTCALL_GCC const char* FASTCALL_MSVC KernelGetErrorMessage(const bool acquire);

FASTCALL_GCC static inline const char* FASTCALL_MSVC KernelGetErrorMessageSafe(const bool acquire)
{
    const char* const message = KernelGetErrorMessage(acquire);
    return message ? message : "";
}

#ifdef __cplusplus
} /* extern "C" */
#endif
