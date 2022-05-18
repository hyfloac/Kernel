#include "kerror.h"

FASTCALL_GCC const char* FASTCALL_MSVC KernelLocalizeErrorCode(const KError_t error)
{
    if(error == 0)
    {
        return "Ok";
    }

    if(error < 0)
    {
        switch(error)
        {
            case KE_FAIL: return "Fail";
            case KE_INVALID_ARG: return "Invalid Arg";
            case KE_PAGING_NO_VALID_PAGES: return "No Valid Pages";
            case KE_PAGING_NO_32_BIT_PAGES: return "No 32-Bit Pages";
            case KE_PAGING_NO_MORE_PLL: return "No More Page Linked List Nodes";
            default: return "Unknown Error";
        }
    }
    else
    {
        // const i16 errorNamespace = error >> 16;
        return "Namespaced errors are not implemented yet.";
    }
}

FASTCALL_GCC void FASTCALL_MSVC KernelRegisterErrorLocalizer(const i16 errorNamespace, const char*(FASTCALL_GCC * callback FASTCALL_MSVC)(KError_t))
{
    (void) errorNamespace;
    (void) callback;
}

static const char* kernelErrorMessage = NULL;
static int kernelErrorFreeOnChange = 0;

FASTCALL_GCC void FASTCALL_MSVC KernelSetErrorMessage(const char* const message, const int freeOnChange)
{
    if(kernelErrorFreeOnChange)
    {
        // free(kerenlErrorMessage);
    }

    kernelErrorMessage = message;
    kernelErrorFreeOnChange = freeOnChange;
}

FASTCALL_GCC const char* FASTCALL_MSVC KernelGetErrorMessage(const int acquire)
{
    if(acquire)
    {
        if(kernelErrorFreeOnChange)
        {
            kernelErrorFreeOnChange = 0;
            const char* const ret = kernelErrorMessage;
            kernelErrorMessage = NULL;
            return ret;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return kernelErrorMessage;
    }
}
