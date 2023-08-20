#include "driver.h"
#include "keyboard.h"
#include "kalloc.h"

static u32 currentDeviceId = 1;

KError_t CreatePhysicalDevice(PhysicalDeviceObject** pdo)
{
    if(!pdo)
    {
        KernelSetErrorMessage("[driver]: pdo was null.", 0);
        return KE_INVALID_ARG;
    }

    *pdo = kalloc(sizeof(PhysicalDeviceObject));
    (*pdo)->Id = currentDeviceId++;
    (*pdo)->Driver = NULL;
    (*pdo)->ManagerAddDevice = NULL;
    (*pdo)->ManagerRemoveDevice = NULL;
    (*pdo)->ManagerStartDevice = NULL;
    (*pdo)->ManagerStopDevice = NULL;

    return KE_OK;
}

KError_t RegisterKMDriver(PhysicalDeviceObject* const pdo, const void* const registration)
{
    if(!pdo)
    {
        KernelSetErrorMessage("[driver]: pdo was null.", 0);
        return KE_INVALID_ARG;
    }

    if(!registration)
    {
        KernelSetErrorMessage("[driver]: registration was null.", 0);
        return KE_INVALID_ARG;
    }

    const KMDriverRegistrationBase* const regBase = (const KMDriverRegistrationBase*) registration;

    switch(regBase->Type)
    {
        case DRIVER_TYPE_KEYBOARD: return KeyboardRegisterKMDriver(pdo, registration);
        default: break;
    }

    KernelSetErrorMessage("[driver]: Driver type was not recognized.", 0);
    return KE_DRIVER_TYPE_NOT_FOUND;
}
