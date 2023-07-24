#pragma once

#include "kstdint.h"
#include "driver.h"
#include "kassert.h"

#define KEY_PRESSED  (0x80000000)
#define KEY_RELEASED (0x00000000)

#define KEY_SHIFT_ACTIVE (0x40000000)
#define KEY_CONTROL_ACTIVE (0x20000000)
#define KEY_ALT_ACTIVE (0x10000000)
#define KEY_COMMAND_ACTIVE (0x08000000)

#define KEY_CODE_MASK (KEY_PRESSED | KEY_SHIFT_ACTIVE | KEY_CONTROL_ACTIVE | KEY_ALT_ACTIVE | KEY_COMMAND_ACTIVE)

#define KEY_IS_PRESSED(Key) (((Key) & KEY_PRESSED) != KEY_PRESSED)
#define KEY_CODE(Key) ((Key) & ~KEY_CODE_MASK)

enum KeyCode
{
    KEY_CODE_UNKNOWN       = 0,
    KEY_CODE_ESC           = 1,
    KEY_CODE_INSERT        = 2,
    KEY_CODE_DELETE        = 3,
    KEY_CODE_BACKSPACE     = 4,
    KEY_CODE_HOME          = 5,
    KEY_CODE_END           = 6,
    KEY_CODE_PAGE_UP       = 7,
    KEY_CODE_PAGE_DOWN     = 8,
    KEY_CODE_TAB           = 9,
    KEY_CODE_ENTER         = 10,
    KEY_CODE_CAPS_LOCK     = 11,
    KEY_CODE_SCROLL_LOCK   = 12,
    KEY_CODE_NUM_LOCK      = 13,
    KEY_CODE_LEFT_SHIFT    = 14,
    KEY_CODE_RIGHT_SHIFT   = 15,
    KEY_CODE_LEFT_CONTROL  = 16,
    KEY_CODE_RIGHT_CONTROL = 17,
    KEY_CODE_LEFT_ALT      = 18,
    KEY_CODE_RIGHT_ALT     = 19,
    KEY_CODE_LEFT_COMMAND  = 20,
    KEY_CODE_RIGHT_COMMAND = 21,
    KEY_CODE_ARROW_UP      = 22,
    KEY_CODE_ARROW_DOWN    = 23,
    KEY_CODE_ARROW_LEFT    = 24,
    KEY_CODE_ARROW_RIGHT   = 25,
    KEY_CODE_TILDE         = 26,
    KEY_CODE_0             = 30,
    KEY_CODE_1             = 31,
    KEY_CODE_2             = 32,
    KEY_CODE_3             = 33,
    KEY_CODE_4             = 34,
    KEY_CODE_5             = 35,
    KEY_CODE_6             = 36,
    KEY_CODE_7             = 37,
    KEY_CODE_8             = 38,
    KEY_CODE_9             = 39,
    KEY_CODE_DASH          = 40,
    KEY_CODE_EQUALS        = 41,
    KEY_CODE_LEFT_BRACKET  = 42,
    KEY_CODE_RIGHT_BRACKET = 43,
    KEY_CODE_BACK_SLASH    = 44,
    KEY_CODE_FORWARD_SLASH = 45,
    KEY_CODE_SEMI_COLON    = 46,
    KEY_CODE_QUOTE         = 47,
    KEY_CODE_COMMA         = 48,
    KEY_CODE_PERIOD        = 49,
    KEY_CODE_A             = 50,
    KEY_CODE_B             = 51,
    KEY_CODE_C             = 52,
    KEY_CODE_D             = 53,
    KEY_CODE_E             = 54,
    KEY_CODE_F             = 55,
    KEY_CODE_G             = 56,
    KEY_CODE_H             = 57,
    KEY_CODE_I             = 58,
    KEY_CODE_J             = 59,
    KEY_CODE_K             = 60,
    KEY_CODE_L             = 61,
    KEY_CODE_M             = 62,
    KEY_CODE_N             = 63,
    KEY_CODE_O             = 64,
    KEY_CODE_P             = 65,
    KEY_CODE_Q             = 66,
    KEY_CODE_R             = 67,
    KEY_CODE_S             = 68,
    KEY_CODE_T             = 69,
    KEY_CODE_U             = 70,
    KEY_CODE_V             = 71,
    KEY_CODE_W             = 72,
    KEY_CODE_X             = 73,
    KEY_CODE_Y             = 74,
    KEY_CODE_Z             = 75,
    KEY_CODE_F1            = 81,
    KEY_CODE_F2            = 82,
    KEY_CODE_F3            = 83,
    KEY_CODE_F4            = 84,
    KEY_CODE_F5            = 85,
    KEY_CODE_F6            = 86,
    KEY_CODE_F7            = 87,
    KEY_CODE_F8            = 88,
    KEY_CODE_F9            = 89,
    KEY_CODE_F10           = 90,
    KEY_CODE_F11           = 91,
    KEY_CODE_F12           = 92,
    KEY_CODE_F13           = 93,
    KEY_CODE_F14           = 94,
    KEY_CODE_F15           = 95,
    KEY_CODE_F16           = 96,
    KEY_CODE_F17           = 97,
    KEY_CODE_F18           = 98,
    KEY_CODE_F19           = 99,
    KEY_CODE_F20           = 100,
    KEY_CODE_F21           = 101,
    KEY_CODE_F22           = 102,
    KEY_CODE_F23           = 103,
    KEY_CODE_F24           = 104,
    KEY_CODE_N0            = 110,
    KEY_CODE_N1            = 111,
    KEY_CODE_N2            = 112,
    KEY_CODE_N3            = 113,
    KEY_CODE_N4            = 114,
    KEY_CODE_N5            = 115,
    KEY_CODE_N6            = 116,
    KEY_CODE_N7            = 117,
    KEY_CODE_N8            = 118,
    KEY_CODE_N9            = 119,
    KEY_CODE_N_PLUS        = 120,
    KEY_CODE_N_MINUS       = 121,
    KEY_CODE_N_ENTER       = 122,
    KEY_CODE_N_PERIOD      = 123,

    KEY_CODE_LAST_COMP,
    KEY_CODE_LAST = KEY_CODE_LAST_COMP - 1
};

enum KeyboardLayout
{
    KEYBOARD_LAYOUT_US_QWERTY = 100,
};

typedef union KeyboardToggleBits
{
    struct
    {
        u8 CapsLock : 1;
        u8 ScrollLock : 1;
        u8 NumLock : 1;
        u8 Reserved : 5;
    } __attribute__((packed));
    u8 ToggleBits;
} __attribute__((packed)) KeyboardToggleBits;

STATIC_ASSERT(sizeof(KeyboardToggleBits) == 1, "KeyboardToggleBits was not a single byte in size.");

u32 CodepointFromKeyCode(const u32 keyCode, const u32 keyboardLayout);

typedef u32(* KeyboardReadKeyCode_f)(void);
typedef b8(* KeyboardIsKeyPressed_f)(const u32 keyCode);
typedef KeyboardToggleBits(* KeyboardGetToggleKeys_f)(void);

typedef struct KMKeyboardDriverRegistrationInfo
{
    KMDriverRegistrationBase Base;
    KeyboardReadKeyCode_f ReadKeyCode;
    KeyboardIsKeyPressed_f IsKeyPressed;
    KeyboardGetToggleKeys_f GetToggleKeys;
} KMKeyboardDriverRegistrationInfo;

#define KM_KEYBOARD_DRIVER_VERSION_0 (0)
#define KM_KEYBOARD_DRIVER_VERSION_CURRENT (KM_KEYBOARD_DRIVER_VERSION_0)

KError_t KeyboardRegisterKMDriver(const void* const registration);

u32 KeyboardReadKeyCode(void);
b8 KeyboardIsKeyPressed(const u32 keyCode);
KeyboardToggleBits KeyboardGetToggleKeys(void);
