#pragma once

#include "driver.h"
#include "kstdint.h"

typedef struct KMAcpiDriverRegistrationInfo
{
    KMDriverRegistrationBase Base;
    KMDriverCoreFunctions CoreFunctions;
} KMAcpiDriverRegistrationInfo;

#define KM_ACPI_DRIVER_VERSION_0 (0)
#define KM_ACPI_DRIVER_VERSION_CURRENT (KM_ACPI_DRIVER_VERSION_0)

KError_t AcpiRegisterKMDriver(PhysicalDeviceObject* const pdo, const void* const registration);

#pragma pack(push, 1)
typedef struct {
    char Signature[8];
    u8 Checksum;
    char OEMID[6];
    u8 Revision;
    u32 RsdtAddress;
} RsdpDescriptor1_0;

typedef struct {
    RsdpDescriptor1_0 Descriptor1_0;
    u32 Length;
    u64 XsdtAddress;
    u8 ExtendedChecksum;
    u8 Reserved[3];
} RsdpDescriptor2_0;
#pragma pack(pop)
