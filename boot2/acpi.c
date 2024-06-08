#include "acpi.h"
#include "kerror.h"

KError_t AcpiRegisterKMDriver(PhysicalDeviceObject* const pdo, const void* const registration)
{
    if(!pdo)
    {
        KernelSetErrorMessage("[acpi_driver]: pdo was null.", false);
        return KE_INVALID_ARG;
    }

    if(!registration)
    {
        KernelSetErrorMessage("[acpi_driver]: registration was null.", false);
        return KE_INVALID_ARG;
    }

    const KMAcpiDriverRegistrationInfo* keyboardRegistration = (const KMAcpiDriverRegistrationInfo*) registration;

    if(keyboardRegistration->Base.Type != DRIVER_TYPE_ACPI)
    {
        KernelSetErrorMessage("[acpi_driver]: Registration type was not DRIVER_TYPE_ACPI.", false);
        return KE_DRIVER_TYPE_MISMATCH;
    }

    if(keyboardRegistration->Base.Size > sizeof(KMAcpiDriverRegistrationInfo))
    {
        KernelSetErrorMessage("[acpi_driver]: Registration was larger than the registration size of the structure this system was built for.", false);
        return KE_DRIVER_SYSTEM_TOO_OLD;
    }

    if(keyboardRegistration->Base.Version > KM_ACPI_DRIVER_VERSION_CURRENT)
    {
        KernelSetErrorMessage("[acpi_driver]: Registration version was newer than the registration version of the structure this system was built for.", false);
        return KE_DRIVER_SYSTEM_TOO_OLD;
    }
    
    return KE_OK;
}
