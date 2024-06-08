#include "keyboard.h"
#include "ps2.h"
#include "kalloc.h"
#include "kstring.h"
#include "memcpy.h"
#include "kprintf.h"
#include "kmaths.h"
#include "pool_allocator.h"
#include "paging.h"

#define POOL_TAG_KEYBOARD_CALLBACKS ('CdbK')
#define POOL_TAG_KEYBOARD_DRIVERS ('DdbK')

typedef struct KMKeyboardDriverList
{
    PhysicalDeviceObject* PDO;
    KMKeyboardDriverRegistrationInfo Registration;
    void* DeviceContext;

    // This should probably be an AVL tree for perf.
    struct KMKeyboardDriverList* pNext;
} KMKeyboardDriverList;

static KMKeyboardDriverList* KeyboardDriverList = NULL;
static KMKeyboardDriverList* KeyboardDriverListTail = NULL;

// Keyboard Callback AVL Node
typedef struct KCAVLNode
{
    struct KCAVLNode* Left;
    struct KCAVLNode* Right;
    KeyboardKeyCallback_f Callback;
    i32 Priority;
    u8 Height;
} KCAVLNode;

static KCAVLNode* KeyboardCallbackRoot = NULL;

static KCAVLNode* KeyboardCallbackAllocList = NULL;

void InitKeyboard(void)
{
    const u32 keyboardCallbackCount = 32;

    KeyboardCallbackAllocList = AllocatePoolWithTag(PoolFlagNonPaged, sizeof(KCAVLNode) * keyboardCallbackCount, POOL_TAG_KEYBOARD_CALLBACKS);

    for(u32 i = 0; i < keyboardCallbackCount - 1; ++i)
    {
        KeyboardCallbackAllocList[i].Left = &KeyboardCallbackAllocList[i + 1];
    }

    KeyboardCallbackAllocList[keyboardCallbackCount - 1].Left = NULL;
}

static u32 CodepointFromKeyCodeUsQwerty(PhysicalDeviceObject* const pdo, const u32 keycode);

u32 CodepointFromKeyCode(PhysicalDeviceObject* const pdo, const u32 keyCode, const u32 keyboardLayout)
{
    switch(keyboardLayout)
    {
        case KEYBOARD_LAYOUT_US_QWERTY: return CodepointFromKeyCodeUsQwerty(pdo, keyCode);
        default: return 0;
    }
}

static u32 LetterFromKeyCodeUsQwerty(const u32 rawKeyCode)
{
    if(rawKeyCode >= KEY_CODE_A && rawKeyCode <= KEY_CODE_Z)
    {
        return rawKeyCode - KEY_CODE_A + 'a';
    }

    return 0;
}

static u32 CodepointFromKeyCodeUsQwerty(PhysicalDeviceObject* const pdo, const u32 keyCode)
{
    const u32 rawKeyCode = KEY_CODE(keyCode);

    if(KeyboardGetToggleKeys(pdo).NumLock && rawKeyCode >= KEY_CODE_N0 && rawKeyCode <= KEY_CODE_N9)
    {
        return rawKeyCode - KEY_CODE_N0 + '0';
    }

    switch(rawKeyCode)
    {
        case KEY_CODE_TAB: return (u32) '\t';
        case KEY_CODE_ENTER: return (u32) '\n';
        case KEY_CODE_SPACE: return (u32) ' ';
        default: break;
    }

    const u32 letter = LetterFromKeyCodeUsQwerty(rawKeyCode);

    if((keyCode & KEY_SHIFT_ACTIVE) == KEY_SHIFT_ACTIVE)
    {
        switch(rawKeyCode)
        {
            case KEY_CODE_TILDE:         return (u32) '~';
            case KEY_CODE_0:             return (u32) ')';
            case KEY_CODE_1:             return (u32) '!';
            case KEY_CODE_2:             return (u32) '@';
            case KEY_CODE_3:             return (u32) '#';
            case KEY_CODE_4:             return (u32) '$';
            case KEY_CODE_5:             return (u32) '%';
            case KEY_CODE_6:             return (u32) '^';
            case KEY_CODE_7:             return (u32) '&';
            case KEY_CODE_8:             return (u32) '*';
            case KEY_CODE_9:             return (u32) '(';
            case KEY_CODE_DASH:          return (u32) '_';
            case KEY_CODE_EQUALS:        return (u32) '+';
            case KEY_CODE_LEFT_BRACKET:  return (u32) '{';
            case KEY_CODE_RIGHT_BRACKET: return (u32) '}';
            case KEY_CODE_BACK_SLASH:    return (u32) '|';
            case KEY_CODE_SEMI_COLON:    return (u32) ':';
            case KEY_CODE_QUOTE:         return (u32) '\"';
            case KEY_CODE_COMMA:         return (u32) '<';
            case KEY_CODE_PERIOD:        return (u32) '>';
            case KEY_CODE_FORWARD_SLASH: return (u32) '?';
            default: break;
        }

        if(letter)
        {
            if(KeyboardGetToggleKeys(pdo).CapsLock)
            {
                return letter;
            }
            else
            {
                return letter - 'a' + 'A';
            }
        }
    }
    else
    {
        if(rawKeyCode >= KEY_CODE_0 && rawKeyCode <= KEY_CODE_9)
        {
            return rawKeyCode - KEY_CODE_0 + '0';
        }

        switch(rawKeyCode)
        {
            case KEY_CODE_TILDE:         return (u32) '`';
            case KEY_CODE_DASH:          return (u32) '-';
            case KEY_CODE_EQUALS:        return (u32) '=';
            case KEY_CODE_LEFT_BRACKET:  return (u32) '[';
            case KEY_CODE_RIGHT_BRACKET: return (u32) ']';
            case KEY_CODE_BACK_SLASH:    return (u32) '\\';
            case KEY_CODE_SEMI_COLON:    return (u32) ';';
            case KEY_CODE_QUOTE:         return (u32) '\'';
            case KEY_CODE_COMMA:         return (u32) ',';
            case KEY_CODE_PERIOD:        return (u32) '.';
            case KEY_CODE_FORWARD_SLASH: return (u32) '/';
            default: break;
        }

        if(letter)
        {
            if(KeyboardGetToggleKeys(pdo).CapsLock)
            {
                return letter - 'a' + 'A';
            }
            else
            {
                return letter;
            }
        }
    }

    return 0;
}

static KError_t KeyboardKMDriverAddDevice(void* driver)
{
    if(!driver)
    {
        KernelSetErrorMessage("[keyboard_driver]: driver was null.", 0);
        return KE_INVALID_ARG;
    }

    KMKeyboardDriverList* driverReal = driver;

    if(!driverReal->Registration.CoreFunctions.AddDevice)
    {
        return KE_OK;
    }

    return driverReal->Registration.CoreFunctions.AddDevice(driverReal->PDO, &driverReal->DeviceContext);
}

static KError_t KeyboardKMDriverRemoveDevice(void* driver)
{
    if(!driver)
    {
        KernelSetErrorMessage("[keyboard_driver]: driver was null.", 0);
        return KE_INVALID_ARG;
    }

    KMKeyboardDriverList* driverReal = driver;

    if(!driverReal->Registration.CoreFunctions.RemoveDevice)
    {
        return KE_OK;
    }

    return driverReal->Registration.CoreFunctions.RemoveDevice(driverReal->DeviceContext);
}

static KError_t KeyboardKMDriverStartDevice(void* driver)
{
    if(!driver)
    {
        KernelSetErrorMessage("[keyboard_driver]: driver was null.", 0);
        return KE_INVALID_ARG;
    }

    KMKeyboardDriverList* driverReal = driver;

    if(!driverReal->Registration.CoreFunctions.StartDevice)
    {
        return KE_OK;
    }

    return KE_OK;

    return driverReal->Registration.CoreFunctions.StartDevice(driverReal->DeviceContext);
}

static KError_t KeyboardKMDriverStopDevice(void* driver)
{
    if(!driver)
    {
        KernelSetErrorMessage("[keyboard_driver]: driver was null.", 0);
        return KE_INVALID_ARG;
    }

    KMKeyboardDriverList* driverReal = driver;

    if(!driverReal->Registration.CoreFunctions.StopDevice)
    {
        return KE_OK;
    }

    return driverReal->Registration.CoreFunctions.StopDevice(driverReal->DeviceContext);
}

KError_t KeyboardRegisterKMDriver(PhysicalDeviceObject* const pdo, const void* const registration)
{
    if(!pdo)
    {
        KernelSetErrorMessage("[keyboard_driver]: pdo was null.", 0);
        return KE_INVALID_ARG;
    }

    if(!registration)
    {
        KernelSetErrorMessage("[keyboard_driver]: registration was null.", 0);
        return KE_INVALID_ARG;
    }

    const KMKeyboardDriverRegistrationInfo* keyboardRegistration = (const KMKeyboardDriverRegistrationInfo*) registration;

    if(keyboardRegistration->Base.Type != DRIVER_TYPE_KEYBOARD)
    {
        KernelSetErrorMessage("[keyboard_driver]: Registration type was not DRIVER_TYPE_KEYBOARD.", 0);
        return KE_DRIVER_TYPE_MISMATCH;
    }

    if(keyboardRegistration->Base.Size > sizeof(KMKeyboardDriverRegistrationInfo))
    {
        KernelSetErrorMessage("[keyboard_driver]: Registration was larger than the registration size of the structure this system was built for.", 0);
        return KE_DRIVER_SYSTEM_TOO_OLD;
    }

    if(keyboardRegistration->Base.Version > KM_KEYBOARD_DRIVER_VERSION_CURRENT)
    {
        KernelSetErrorMessage("[keyboard_driver]: Registration version was newer than the registration version of the structure this system was built for.", 0);
        return KE_DRIVER_SYSTEM_TOO_OLD;
    }
    
    u32 allocSize = sizeof(KMKeyboardDriverList);

    if(keyboardRegistration->Base.pNext)
    {
        const KMDriverRegistrationBase* curr = (const KMDriverRegistrationBase*) keyboardRegistration->Base.pNext;

        do
        {
            switch(curr->Type)
            {
                case DRIVER_TYPE_INFO_ATTACHMENT:
                    if(curr->Size > sizeof(KMDriverInfoAttachment))
                    {
                        KernelSetErrorMessage("[keyboard_driver]: Registration extension KMDriverInfoAttachment was larger than the registration size of the structure this system was built for.", 0);
                        return KE_DRIVER_SYSTEM_TOO_OLD;
                    }

                    if(curr->Version > KM_DRIVER_INFO_ATTACHMENT_VERSION_CURRENT)
                    {
                        KernelSetErrorMessage("[keyboard_driver]: Registration extension KMDriverInfoAttachment version was newer than the registration version of the structure this system was built for.", 0);
                        return KE_DRIVER_SYSTEM_TOO_OLD;
                    }

                    allocSize += sizeof(KMDriverInfoAttachment);
                    allocSize += StrLen(((const KMDriverInfoAttachment*) curr)->Name);
                    break;
                default:
                    KernelSetErrorMessage("[keyboard_driver]: Did not recognize extension structure.", 0);
                    return KE_DRIVER_TYPE_NOT_FOUND;
            }

            curr = (const KMDriverRegistrationBase*) curr->pNext;
        } while (curr);
    }

    KMKeyboardDriverList* driverInfo = AllocatePoolWithTag(PoolFlagNonPaged | PoolFlagZeroMemory, allocSize, POOL_TAG_KEYBOARD_DRIVERS);

    pdo->Driver = driverInfo;
    pdo->ManagerAddDevice = KeyboardKMDriverAddDevice;
    pdo->ManagerRemoveDevice = KeyboardKMDriverRemoveDevice;
    pdo->ManagerStartDevice = KeyboardKMDriverStartDevice;
    pdo->ManagerStopDevice = KeyboardKMDriverStopDevice;

    driverInfo->PDO = pdo;
    // This is a nullptr, except it will actually trigger a page fault.
    driverInfo->DeviceContext = (void*) 0x70000000;

    memcpy(&driverInfo->Registration, keyboardRegistration, sizeof(*keyboardRegistration));
    driverInfo->Registration.Base.pNext = nullptr;

    // driverInfo->Registration.Base.Type = keyboardRegistration->Base.Type;
    // driverInfo->Registration.Base.Size = keyboardRegistration->Base.Size;
    // driverInfo->Registration.Base.Version = keyboardRegistration->Base.Version;
    // driverInfo->Registration.Base.pNext = NULL;
    // driverInfo->Registration.CoreFunctions.AddDevice = keyboardRegistration->CoreFunctions.AddDevice;
    // driverInfo->Registration.CoreFunctions.RemoveDevice = keyboardRegistration->CoreFunctions.RemoveDevice;
    // driverInfo->Registration.CoreFunctions.StartDevice = keyboardRegistration->CoreFunctions.StartDevice;
    // driverInfo->Registration.CoreFunctions.StopDevice = keyboardRegistration->CoreFunctions.StopDevice;
    // driverInfo->Registration.ReadKeyCode = keyboardRegistration->ReadKeyCode;
    // driverInfo->Registration.IsKeyPressed = keyboardRegistration->IsKeyPressed;
    // driverInfo->Registration.GetToggleKeys = keyboardRegistration->GetToggleKeys;

    if(keyboardRegistration->Base.pNext)
    {
        void* currAllocation = driverInfo + 1;
        const void** currPNext = &driverInfo->Registration.Base.pNext;
        const KMDriverRegistrationBase* curr = (const KMDriverRegistrationBase*) keyboardRegistration->Base.pNext;

        do
        {
            switch(curr->Type)
            {
                case DRIVER_TYPE_INFO_ATTACHMENT:
                {
                    const KMDriverInfoAttachment* currInfoAttachment = (const KMDriverInfoAttachment*) curr;
                    KMDriverInfoAttachment* infoAttachment = currAllocation;
                    currAllocation = infoAttachment + 1;

                    memcpy(infoAttachment, currInfoAttachment, sizeof(*infoAttachment));
                    infoAttachment->Base.pNext = nullptr;

                    const u32 nameLen = StrLen(currInfoAttachment->Name);

                    char* nameAllocation = currAllocation;
                    currAllocation = nameAllocation + nameLen + 1;
                    memcpy(nameAllocation, currInfoAttachment->Name, nameLen + 1);

                    *currPNext = infoAttachment;
                    currPNext = &infoAttachment->Base.pNext;

                    // infoAttachment->Base.Type = currInfoAttachment->Base.Type;
                    // infoAttachment->Base.Size = currInfoAttachment->Base.Size;
                    // infoAttachment->Base.Version = currInfoAttachment->Base.Version;
                    // infoAttachment->Name = nameAllocation;
                    // infoAttachment->Verison = currInfoAttachment->Verison;
                    
                    break;
                }
                default: break;
            }

            curr = (const KMDriverRegistrationBase*) curr->pNext;
        } while (curr);
    }

    if(!KeyboardDriverList)
    {
        KeyboardDriverList = driverInfo;
        KeyboardDriverListTail = driverInfo;
    }
    else
    {
        KeyboardDriverListTail->pNext = driverInfo;
        KeyboardDriverListTail = driverInfo;
    }
    
    return KE_OK;
}

KMKeyboardDriverList* FindKeyboardDriver(PhysicalDeviceObject* pdo)
{
    KMKeyboardDriverList* curr = KeyboardDriverList;

    while(curr)
    {
        if(curr->PDO == pdo)
        {
            return curr;
        }

        curr = curr->pNext;
    }

    return NULL;
}

u32 KeyboardReadKeyCode(PhysicalDeviceObject* const pdo)
{
    KMKeyboardDriverList* driver = FindKeyboardDriver(pdo);

    if(driver)
    {
        return driver->Registration.ReadKeyCode(driver->DeviceContext);
    }

    return 0;
}

b8 KeyboardIsKeyPressed(PhysicalDeviceObject* const pdo, const u32 keyCode)
{
    KMKeyboardDriverList* driver = FindKeyboardDriver(pdo);

    if(driver)
    {
        return driver->Registration.IsKeyPressed(driver->DeviceContext, keyCode);
    }

    return 0;
}

KeyboardToggleBits KeyboardGetToggleKeys(PhysicalDeviceObject* const pdo)
{
    KMKeyboardDriverList* driver = FindKeyboardDriver(pdo);

    if(driver)
    {
        return driver->Registration.GetToggleKeys(driver->DeviceContext);
    }

    KeyboardToggleBits bits;
    bits.ToggleBits = 0;

    return bits;
}

static KCAVLNode* KeyboardInsertCallback(const KeyboardKeyCallback_f callback, const i32 priority);

KError_t KeyboardRegisterKeyCallback(const KeyboardKeyCallback_f callback, const i32 priority)
{
    KCAVLNode* const node = KeyboardInsertCallback(callback, priority);

    if(!node)
    {
        return KE_OUT_OF_MEMORY;
    }

    return KE_OK;
}

static b8 KeyboardNotifyKeyEventIterate(PhysicalDeviceObject* const pdo, const u32 keyCode, const KeyboardToggleBits toggleBits, const KCAVLNode* const tree)
{
    if(!tree)
    {
        return false;
    }

    if(KeyboardNotifyKeyEventIterate(pdo, keyCode, toggleBits, tree->Left))
    {
        return true;
    }

    if(tree->Callback)
    {
        if(tree->Callback(pdo, keyCode, toggleBits))
        {
            return true;
        }
    }

    return KeyboardNotifyKeyEventIterate(pdo, keyCode, toggleBits, tree->Right);
}

void KeyboardNotifyKeyEvent(PhysicalDeviceObject* const pdo)
{
    const u32 keyCode = KeyboardReadKeyCode(pdo);
    const KeyboardToggleBits toggleBits = KeyboardGetToggleKeys(pdo);

    KeyboardNotifyKeyEventIterate(pdo, keyCode, toggleBits, KeyboardCallbackRoot);
}

static KCAVLNode* KeyboardAllocCallbackNode()
{
    if(!KeyboardCallbackAllocList)
    {
        return NULL;
    }

    KCAVLNode* const ret = KeyboardCallbackAllocList;
    KeyboardCallbackAllocList = KeyboardCallbackAllocList->Left;

    return ret;
}

// static void KeyboardFreeCallbackNode(KCAVLNode* const node)
// {
//     if(!node)
//     {
//         return;
//     }

//     node->Left = KeyboardCallbackAllocList;
//     KeyboardCallbackAllocList = node;   
// }

static u8 KeyboardAVLHeight(const KCAVLNode* tree)
{
    if(!tree)
    {
        return 0;
    }
    return tree->Height;
}

static i32 KeyboardAVLComputeBalance(const KCAVLNode* const tree)
{
    if(!tree)
    { 
        return 0; 
    }

    return (i32) KeyboardAVLHeight(tree->Left) - (i32) KeyboardAVLHeight(tree->Right);
}

static KCAVLNode* KeyboardAVLRotateRight(KCAVLNode* const pivot)
{
    KCAVLNode* const newRoot = pivot->Left;
    KCAVLNode* const transferNode = newRoot->Right;

    newRoot->Right = pivot;
    pivot->Left = transferNode;

    pivot->Height = KM_MAX(KeyboardAVLHeight(pivot->Left), KeyboardAVLHeight(pivot->Right)) + 1;
    newRoot->Height = KM_MAX(KeyboardAVLHeight(newRoot->Left), KeyboardAVLHeight(newRoot->Right)) + 1;

    return newRoot;
}

static KCAVLNode* KeyboardAVLRotateLeft(KCAVLNode* const pivot)
{
    KCAVLNode* const newRoot = pivot->Right;
    KCAVLNode* const transferNode = newRoot->Left;

    newRoot->Left = pivot;
    pivot->Right = transferNode;

    pivot->Height = KM_MAX(KeyboardAVLHeight(pivot->Left), KeyboardAVLHeight(pivot->Right)) + 1;
    newRoot->Height = KM_MAX(KeyboardAVLHeight(newRoot->Left), KeyboardAVLHeight(newRoot->Right)) + 1;

    return newRoot;
}

static KCAVLNode* KeyboardAVLInsertCallback(KCAVLNode* tree, KCAVLNode* newNode)
{
    if(!tree)
    {
        return newNode;
    }

    if(newNode->Priority <= tree->Priority)
    {
        tree->Left = KeyboardAVLInsertCallback(tree->Left, newNode);
    }
    else if(newNode->Priority > tree->Priority)
    {
        tree->Right = KeyboardAVLInsertCallback(tree->Right, newNode);
    }

    tree->Height = KM_MAX(KeyboardAVLHeight(tree->Left), KeyboardAVLHeight(tree->Right)) + 1;
    const i32 balance = KeyboardAVLComputeBalance(tree);

    // Left Left
    if(balance > 1 && newNode->Priority < tree->Left->Priority)
    { 
        return KeyboardAVLRotateRight(tree); 
    }

    // Right Right
    if(balance < -1 && newNode->Priority > tree->Right->Priority)
    { 
        return KeyboardAVLRotateLeft(tree);
    }

    // Left Right
    if(balance > 1 && newNode->Priority > tree->Left->Priority)
    {
        tree->Left = KeyboardAVLRotateLeft(tree->Left);
        return KeyboardAVLRotateRight(tree);
    }

    // Right Left
    if(balance < -1 && newNode->Priority < tree->Right->Priority)
    {
        tree->Right = KeyboardAVLRotateRight(tree->Right);
        return KeyboardAVLRotateLeft(tree);
    }

    return tree;
}

static KCAVLNode* KeyboardInsertCallback(const KeyboardKeyCallback_f callback, const i32 priority)
{
    KCAVLNode* const node = KeyboardAllocCallbackNode();

    if(!node)
    {
        return node;
    }

    node->Callback = callback;
    node->Priority = priority;
    KeyboardCallbackRoot = KeyboardAVLInsertCallback(KeyboardCallbackRoot, node);

    return node;
}

