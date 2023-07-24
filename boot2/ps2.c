#include "ps2.h"
#include "ps2.private.h"
#include "io.h"
#include "keyboard.h"
#include "memset.h"

static u32 PS2ActiveKeyMask[KEY_CODE_LAST / (sizeof(u32) * CHAR_BIT) + 1];
static KeyboardToggleBits PS2ToggleBits;

#define CAPS_LOCK_BIT (0x01)
#define SCROLL_LOCK_BIT (0x02)
#define NUM_LOCK_BIT (0x04)

int PS2Init(void)
{
    zeromem_stosb(PS2ActiveKeyMask, sizeof(PS2ActiveKeyMask));
    PS2ToggleBits.ToggleBits = 0;

    return 0;
}

u8 PS2ReadData(void)
{
    return in8(PS2_DATA);
}

#define KEY_MATCHER_SCS1(Key) \
    case PS2_SCS1_##Key##_PRESSED:  keyCode = KEY_CODE_##Key | KEY_PRESSED; break; \
    case PS2_SCS1_##Key##_RELEASED: keyCode = KEY_CODE_##Key | KEY_RELEASED; break

#define KEY_MATCHER_SCS1_BYTE1(Key) \
    case PS2_SCS1_##Key##_PRESSED_BYTE_1:  keyCode = KEY_CODE_##Key | KEY_PRESSED; break; \
    case PS2_SCS1_##Key##_RELEASED_BYTE_1: keyCode = KEY_CODE_##Key | KEY_RELEASED; break


static void PS2UpdateActiveKeyMask(const u32 keyCode)
{
    const u32 divisor = sizeof(u32) * CHAR_BIT;

    const u32 rawKeyCode = KEY_CODE(keyCode);

    const u32 wordIndex = rawKeyCode / divisor;
    const u32 bitIndex = rawKeyCode % divisor;

    if(KEY_IS_PRESSED(keyCode))
    {
        PS2ActiveKeyMask[wordIndex] |= bitIndex;

        if(rawKeyCode == KEY_CODE_CAPS_LOCK)
        {
            PS2ToggleBits.CapsLock ^= 1;
        }
        else if(rawKeyCode == KEY_CODE_SCROLL_LOCK)
        {
            PS2ToggleBits.ScrollLock ^= 1;
        }
        else if(rawKeyCode == KEY_CODE_NUM_LOCK)
        {
            PS2ToggleBits.NumLock ^= 1;
        }
    }
    else
    {
        PS2ActiveKeyMask[wordIndex] &= ~bitIndex;
    }
}

u32 PS2ReadKeyCode(void)
{
    const u8 byte0 = PS2ReadData();

    u32 keyCode = KEY_CODE_UNKNOWN;

    switch(byte0)
    {
        KEY_MATCHER_SCS1(ESC);
        KEY_MATCHER_SCS1(BACKSPACE);

        KEY_MATCHER_SCS1(ENTER);
        
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

    PS2UpdateActiveKeyMask(keyCode);

    return keyCode;
}

b8 PS2IsKeyPressed(const u32 keyCode)
{
    const u32 divisor = sizeof(u32) * CHAR_BIT;

    const u32 rawKeyCode = KEY_CODE(keyCode);

    const u32 wordIndex = rawKeyCode / divisor;
    const u32 bitIndex = rawKeyCode % divisor;

    return (PS2ActiveKeyMask[wordIndex] & bitIndex) != 0;
}

KeyboardToggleBits PS2GetKeyboardToggleBits(void)
{
    return PS2ToggleBits;
}

b8 PS2IsCapsLockActive(void)
{
    return PS2ToggleBits.CapsLock;
}

b8 PS2IsScrollLockActive(void)
{
    return PS2ToggleBits.ScrollLock;
}

b8 PS2IsNumLockActive(void)
{
    return PS2ToggleBits.NumLock;
}

KError_t RegisterPS2KeyboardDriver(void)
{
    KMKeyboardDriverRegistrationInfo registration = { '\0' };
    registration.Base.Type = DRIVER_TYPE_KEYBOARD;
    registration.Base.Size = sizeof(registration);
    registration.Base.Version = KM_KEYBOARD_DRIVER_VERSION_CURRENT;
    registration.ReadKeyCode = PS2ReadKeyCode;
    registration.IsKeyPressed = PS2IsKeyPressed;
    registration.GetToggleKeys = PS2GetKeyboardToggleBits;

    KMDriverInfoAttachment infoAttachment = { '\0' };
    infoAttachment.Base.Type = DRIVER_TYPE_INFO_ATTACHMENT;
    infoAttachment.Base.Size = sizeof(infoAttachment);
    infoAttachment.Base.Version = KM_DRIVER_INFO_ATTACHMENT_VERSION_CURRENT;
    infoAttachment.Base.pNext = NULL;
    infoAttachment.Name = "PS/2 Keyboard Driver";
    infoAttachment.Verison = KM_DRIVER_INFO_ATTACHMENT_MAKE_VERSION(1, 0, 0);

    registration.Base.pNext = &infoAttachment;

    return RegisterKMDriver(&registration);
}
