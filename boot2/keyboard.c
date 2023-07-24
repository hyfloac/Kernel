#include "keyboard.h"
#include "ps2.h"
#include "kalloc.h"
#include "kstring.h"
#include "memcpy.h"
#include "kprintf.h"

static u32 CodepointFromKeyCodeUsQwerty(const u32 keycode);

u32 CodepointFromKeyCode(const u32 keyCode, const u32 keyboardLayout)
{
    switch(keyboardLayout)
    {
        case KEYBOARD_LAYOUT_US_QWERTY: return CodepointFromKeyCodeUsQwerty(keyCode);
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

static u32 CodepointFromKeyCodeUsQwerty(const u32 keyCode)
{
    const u32 rawKeyCode = KEY_CODE(keyCode);

    if(KeyboardGetToggleKeys().NumLock && rawKeyCode >= KEY_CODE_N0 && rawKeyCode <= KEY_CODE_N9)
    {
        return rawKeyCode - KEY_CODE_N0 + '0';
    }

    switch(rawKeyCode)
    {
        case KEY_CODE_TAB: return (u32) '\t';
        case KEY_CODE_ENTER: return (u32) '\n';
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
            if(KeyboardGetToggleKeys().CapsLock)
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
            if(KeyboardGetToggleKeys().CapsLock)
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

typedef struct KMKeyboardDriverList
{
    KMKeyboardDriverRegistrationInfo Registration;
    // u32 PhysicalDeviceObject; ?

    // This should probably be an AVL tree for perf.
    struct KMKeyboardDriverList* pNext;
} KMKeyboardDriverList;

static KMKeyboardDriverList* KeyboardDriverList = NULL;
static KMKeyboardDriverList* KeyboardDriverListTail = NULL;

KError_t KeyboardRegisterKMDriver(const void* const registration)
{
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
    
    u32 allocSize = sizeof(KMKeyboardDriverRegistrationInfo);

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

    KMKeyboardDriverList* driverInfo = kalloc(allocSize);

    driverInfo->Registration.Base.Type = keyboardRegistration->Base.Type;
    driverInfo->Registration.Base.Size = keyboardRegistration->Base.Size;
    driverInfo->Registration.Base.Version = keyboardRegistration->Base.Version;
    driverInfo->Registration.Base.pNext = NULL;
    driverInfo->Registration.ReadKeyCode = keyboardRegistration->ReadKeyCode;
    driverInfo->Registration.IsKeyPressed = keyboardRegistration->IsKeyPressed;
    driverInfo->Registration.GetToggleKeys = keyboardRegistration->GetToggleKeys;

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

                    const u32 nameLen = StrLen(currInfoAttachment->Name);

                    char* nameAllocation = currAllocation;
                    currAllocation = nameAllocation + nameLen + 1;
                    memcpy(nameAllocation, currInfoAttachment->Name, nameLen + 1);

                    *currPNext = infoAttachment;
                    currPNext = &infoAttachment->Base.pNext;

                    infoAttachment->Base.Type = currInfoAttachment->Base.Type;
                    infoAttachment->Base.Size = currInfoAttachment->Base.Size;
                    infoAttachment->Base.Version = currInfoAttachment->Base.Version;
                    infoAttachment->Name = nameAllocation;
                    infoAttachment->Verison = currInfoAttachment->Verison;
                    
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

u32 KeyboardReadKeyCode(void)
{
    if(KeyboardDriverList)
    {
        return KeyboardDriverList->Registration.ReadKeyCode();
    }

    return 0;
}

b8 KeyboardIsKeyPressed(const u32 keyCode)
{
    if(KeyboardDriverList)
    {
        return KeyboardDriverList->Registration.IsKeyPressed(keyCode);
    }

    return 0;
}

KeyboardToggleBits KeyboardGetToggleKeys(void)
{
    if(KeyboardDriverList)
    {
        return KeyboardDriverList->Registration.GetToggleKeys();
    }

    KeyboardToggleBits bits;
    bits.ToggleBits = 0;

    return bits;
}
