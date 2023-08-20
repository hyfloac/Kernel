#include "ps2.h"
#include "ps2.private.h"
#include "io.h"
#include "keyboard.h"
#include "memset.h"
#include "kalloc.h"

typedef struct PS2KeyboardContext
{
    PhysicalDeviceObject* KeyboardPDO;
    u32 ActiveKeyMask[KEY_CODE_LAST / (sizeof(u32) * CHAR_BIT) + 1];
    KeyboardToggleBits ToggleBits;
    b8 Started;
} PS2KeyboardContext;

// static u32 PS2ActiveKeyMask[KEY_CODE_LAST / (sizeof(u32) * CHAR_BIT) + 1];
// static KeyboardToggleBits PS2ToggleBits;
static PhysicalDeviceObject* PS2KeyboardPDO;

#define CAPS_LOCK_BIT (0x01)
#define SCROLL_LOCK_BIT (0x02)
#define NUM_LOCK_BIT (0x04)

int PS2Init(void)
{
    // zeromem_stosb(PS2ActiveKeyMask, sizeof(PS2ActiveKeyMask));
    // PS2ToggleBits.ToggleBits = 0;

    return 0;
}


static void PS2UpdateActiveKeyMask(const u32 keyCode, PS2KeyboardContext* const context);
static u32 PS2KeyboardReadKeyCode(void* const deviceContext);
static b8 PS2KeyboardIsKeyPressed(void* const deviceContext, const u32 keyCode);
static KeyboardToggleBits PS2KeyboardGetToggleKeys(void* const deviceContext);

u8 PS2ReadData(void)
{
    return in8(PS2_DATA);
}

u32 PS2ReadKeyCode(void)
{
    return KeyboardReadKeyCode(PS2KeyboardPDO);
}

b8 PS2IsKeyPressed(const u32 keyCode)
{
    return KeyboardIsKeyPressed(PS2KeyboardPDO, keyCode);
}

KeyboardToggleBits PS2GetKeyboardToggleBits(void)
{
    return KeyboardGetToggleKeys(PS2KeyboardPDO);
}

static KError_t PS2KeyboardAddDevice(PhysicalDeviceObject* pdo, void** deviceContext)
{
    PS2KeyboardContext* context = kalloc(sizeof(PS2KeyboardContext));
    
    context->KeyboardPDO = pdo;
    context->Started = false;

    *deviceContext = context;

    return KE_OK;
}

static KError_t PS2KeyboardRemoveDevice(void* deviceContext)
{
    kfree(deviceContext);

    return KE_OK;
}

static KError_t PS2KeyboardStartDevice(void* deviceContext)
{
    PS2KeyboardContext* context = (PS2KeyboardContext*) deviceContext;

    zeromem_stosb(context->ActiveKeyMask, sizeof(context->ActiveKeyMask));
    context->ToggleBits.ToggleBits = 0;
    context->Started = true;

    return KE_OK;
}

static KError_t PS2KeyboardStopDevice(void* deviceContext)
{
    PS2KeyboardContext* context = (PS2KeyboardContext*) deviceContext;

    context->Started = false;

    return KE_OK;
}

KError_t RegisterPS2KeyboardDriver(void)
{
    {
        const KError_t error = CreatePhysicalDevice(&PS2KeyboardPDO);
        if(!KE_IS_OK(error))
        {
            return error;
        }
    }

    KMKeyboardDriverRegistrationInfo registration = { '\0' };
    registration.Base.Type = DRIVER_TYPE_KEYBOARD;
    registration.Base.Size = sizeof(registration);
    registration.Base.Version = KM_KEYBOARD_DRIVER_VERSION_CURRENT;
    registration.CoreFunctions.AddDevice = PS2KeyboardAddDevice;
    registration.CoreFunctions.RemoveDevice = PS2KeyboardRemoveDevice;
    registration.CoreFunctions.StartDevice = PS2KeyboardStartDevice;
    registration.CoreFunctions.StopDevice = PS2KeyboardStopDevice;
    registration.ReadKeyCode = PS2KeyboardReadKeyCode;
    registration.IsKeyPressed = PS2KeyboardIsKeyPressed;
    registration.GetToggleKeys = PS2KeyboardGetToggleKeys;

    KMDriverInfoAttachment infoAttachment = { '\0' };
    infoAttachment.Base.Type = DRIVER_TYPE_INFO_ATTACHMENT;
    infoAttachment.Base.Size = sizeof(infoAttachment);
    infoAttachment.Base.Version = KM_DRIVER_INFO_ATTACHMENT_VERSION_CURRENT;
    infoAttachment.Base.pNext = NULL;
    infoAttachment.Name = "PS/2 Keyboard Driver";
    infoAttachment.Verison = KM_DRIVER_INFO_ATTACHMENT_MAKE_VERSION(1, 0, 0);

    registration.Base.pNext = &infoAttachment;

    return RegisterKMDriver(PS2KeyboardPDO, &registration);
}

PhysicalDeviceObject* GetPS2KeyboardPDO(void)
{
    return PS2KeyboardPDO;
}

static void PS2UpdateActiveKeyMask(const u32 keyCode, PS2KeyboardContext* const context)
{
    const u32 divisor = sizeof(u32) * CHAR_BIT;

    const u32 rawKeyCode = KEY_CODE(keyCode);

    const u32 wordIndex = rawKeyCode / divisor;
    const u32 bitIndex = rawKeyCode % divisor;
    const u32 bit = 1 << bitIndex;

    if(KEY_IS_PRESSED(keyCode))
    {
        context->ActiveKeyMask[wordIndex] |= bit;

        if(rawKeyCode == KEY_CODE_CAPS_LOCK)
        {
            context->ToggleBits.CapsLock ^= 1;
        }
        else if(rawKeyCode == KEY_CODE_SCROLL_LOCK)
        {
            context->ToggleBits.ScrollLock ^= 1;
        }
        else if(rawKeyCode == KEY_CODE_NUM_LOCK)
        {
            context->ToggleBits.NumLock ^= 1;
        }
    }
    else
    {
        context->ActiveKeyMask[wordIndex] &= ~bit;
    }
}

static u32 PS2KeyboardReadKeyCode(void* const deviceContext)
{
#define KEY_MATCHER_SCS1(Key) \
    case PS2_SCS1_##Key##_PRESSED:  keyCode = KEY_CODE_##Key | KEY_PRESSED; break; \
    case PS2_SCS1_##Key##_RELEASED: keyCode = KEY_CODE_##Key | KEY_RELEASED; break

#define KEY_MATCHER_SCS1_BYTE1(Key) \
    case PS2_SCS1_##Key##_PRESSED_BYTE_1:  keyCode = KEY_CODE_##Key | KEY_PRESSED; break; \
    case PS2_SCS1_##Key##_RELEASED_BYTE_1: keyCode = KEY_CODE_##Key | KEY_RELEASED; break

    const u8 byte0 = PS2ReadData();

    u32 keyCode = KEY_CODE_UNKNOWN;

    switch(byte0)
    {
        KEY_MATCHER_SCS1(ESC);
        KEY_MATCHER_SCS1(BACKSPACE);

        KEY_MATCHER_SCS1(ENTER);

        KEY_MATCHER_SCS1(LEFT_SHIFT);
        KEY_MATCHER_SCS1(RIGHT_SHIFT);
        
        KEY_MATCHER_SCS1(SPACE);
        KEY_MATCHER_SCS1(0);
        KEY_MATCHER_SCS1(1);
        KEY_MATCHER_SCS1(2);
        KEY_MATCHER_SCS1(3);
        KEY_MATCHER_SCS1(4);
        KEY_MATCHER_SCS1(5);
        KEY_MATCHER_SCS1(6);
        KEY_MATCHER_SCS1(7);
        KEY_MATCHER_SCS1(8);
        KEY_MATCHER_SCS1(9);
        KEY_MATCHER_SCS1(DASH);
        KEY_MATCHER_SCS1(EQUALS);
        
        KEY_MATCHER_SCS1(A);
        KEY_MATCHER_SCS1(B);
        KEY_MATCHER_SCS1(C);
        KEY_MATCHER_SCS1(D);
        KEY_MATCHER_SCS1(E);
        KEY_MATCHER_SCS1(F);
        KEY_MATCHER_SCS1(G);
        KEY_MATCHER_SCS1(H);
        KEY_MATCHER_SCS1(I);
        KEY_MATCHER_SCS1(J);
        KEY_MATCHER_SCS1(K);
        KEY_MATCHER_SCS1(L);
        KEY_MATCHER_SCS1(M);
        KEY_MATCHER_SCS1(N);
        KEY_MATCHER_SCS1(O);
        KEY_MATCHER_SCS1(P);
        KEY_MATCHER_SCS1(Q);
        KEY_MATCHER_SCS1(R);
        KEY_MATCHER_SCS1(S);
        KEY_MATCHER_SCS1(T);
        KEY_MATCHER_SCS1(U);
        KEY_MATCHER_SCS1(V);
        KEY_MATCHER_SCS1(W);
        KEY_MATCHER_SCS1(X);
        KEY_MATCHER_SCS1(Y);
        KEY_MATCHER_SCS1(Z);
        default: break;
    }

    if(byte0 == PS2_SCS1_EXTENDED_BYTE_0)
    {
        const u8 byte1 = PS2ReadData();

        switch(byte1)
        {
            KEY_MATCHER_SCS1_BYTE1(INSERT);
            KEY_MATCHER_SCS1_BYTE1(DELETE);
        }
    }

    PS2UpdateActiveKeyMask(keyCode, deviceContext);

    if(PS2KeyboardIsKeyPressed(deviceContext, KEY_CODE_LEFT_SHIFT) || PS2KeyboardIsKeyPressed(deviceContext, KEY_CODE_RIGHT_SHIFT))
    {
        keyCode |= KEY_SHIFT_ACTIVE;
    }


    return keyCode;
}

static b8 PS2KeyboardIsKeyPressed(void* const deviceContext, const u32 keyCode)
{
    PS2KeyboardContext* const context = deviceContext;

    const u32 divisor = sizeof(u32) * CHAR_BIT;

    const u32 rawKeyCode = KEY_CODE(keyCode);

    const u32 wordIndex = rawKeyCode / divisor;
    const u32 bitIndex = rawKeyCode % divisor;

    return (context->ActiveKeyMask[wordIndex] & (1 << bitIndex)) != 0;
}

static KeyboardToggleBits PS2KeyboardGetToggleKeys(void* const deviceContext)
{
    PS2KeyboardContext* const context = deviceContext;
    return context->ToggleBits;
}