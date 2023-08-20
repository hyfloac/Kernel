#pragma once

#include "kstdint.h"
#include "kerror.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PhysicalDeviceObject;

typedef KError_t(* DriverManagerAddDevice_f)(void* driver);
typedef KError_t(* DriverManagerRemoveDevice_f)(void* driver);
typedef KError_t(* DriverManagerStartDevice_f)(void* driver);
typedef KError_t(* DriverManagerStopDevice_f)(void* driver);

typedef struct PhysicalDeviceObject
{
    u32 Id;
    void* Driver;
    DriverManagerAddDevice_f ManagerAddDevice;
    DriverManagerRemoveDevice_f ManagerRemoveDevice;
    DriverManagerStartDevice_f ManagerStartDevice;
    DriverManagerStopDevice_f ManagerStopDevice;
} PhysicalDeviceObject;

enum KMDriverRegistrationType
{
    DRIVER_TYPE_UNKNOWN = 0,
    DRIVER_TYPE_KEYBOARD = 1,

    DRIVER_TYPE_INFO_ATTACHMENT = 0x60000001,

    DRIVER_TYPE_FORCE_DWORD = 0x7FFFFFFF
};

typedef struct KMDriverRegistrationBase
{
    /// @brief The type of the driver, value should be in KMDriverRegistrationType.
    u32 Type;
    /// @brief The size of this registration structure, including this base.
    u32 Size;
    /// @brief The version of this driver registration structure.
    u32 Version;
    /// @brief An optional attachment.
    const void* pNext;
} KMDriverRegistrationBase;

typedef struct KMDriverInfoAttachment
{
    KMDriverRegistrationBase Base;
    /// @brief The name of this driver. This value does not need to outlast the return from driver registration.
    /// It is incumbent upon driver managers to allocate storage for this value.
    const char* Name;
    /// @brief The version of this driver.
    u32 Verison;
} KMDriverInfoAttachment;

#define KM_DRIVER_INFO_ATTACHMENT_VERSION_0 (0)
#define KM_DRIVER_INFO_ATTACHMENT_VERSION_CURRENT (KM_DRIVER_INFO_ATTACHMENT_VERSION_0)

// u32 KM_DRIVER_INFO_ATTACHMENT_MAKE_VERSION(const u8 MAJOR, const u8 MINOR, const u16 PATCH);
#define KM_DRIVER_INFO_ATTACHMENT_MAKE_VERSION(MAJOR, MINOR, PATCH) ((((u32) (MAJOR)) << 24) | (((u32) (MAJOR)) << 16) | ((u32) (PATCH)))

typedef KError_t(* DriverAddDevice_f)(PhysicalDeviceObject* pdo, void** deviceContext);
typedef KError_t(* DriverRemoveDevice_f)(void* deviceContext);
typedef KError_t(* DriverStartDevice_f)(void* deviceContext);
typedef KError_t(* DriverStopDevice_f)(void* deviceContext);

typedef struct KMDriverCoreFunctions
{
    DriverAddDevice_f AddDevice;
    DriverRemoveDevice_f RemoveDevice;
    DriverStartDevice_f StartDevice;
    DriverStopDevice_f StopDevice;
} KMDriverCoreFunctions;

KError_t CreatePhysicalDevice(PhysicalDeviceObject** pdo);

KError_t RegisterKMDriver(PhysicalDeviceObject* const pdo, const void* const registrationStructure);


#ifdef __cplusplus
} /* extern "C" */
#endif