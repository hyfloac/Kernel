#include "driver.h"
#include "keyboard.h"

KError_t RegisterKMDriver(const void* const registration)
{
    if(!registration)
    {
        KernelSetErrorMessage("[driver]: registration was null.", 0);
        return KE_INVALID_ARG;
    }

    const KMDriverRegistrationBase* const regBase = (const KMDriverRegistrationBase*) registration;

    switch(regBase->Type)
    {
        case DRIVER_TYPE_KEYBOARD: return KeyboardRegisterKMDriver(registration);
        default: break;
    }

    KernelSetErrorMessage("[driver]: Driver type was not recognized.", 0);
    return KE_DRIVER_TYPE_NOT_FOUND;
}
