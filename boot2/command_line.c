#include "command_line.h"
#include "kerror.h"
#include "keyboard.h"
#include "console.h"
#include "pool_allocator.h"
#include "memset.h"
#include "kstring.h"
#include "checkmem.h"

static b8 CommandLineKeyboardCallback(PhysicalDeviceObject* const pdo, const u32 keyCode, const KeyboardToggleBits toggleBits);

#define COMMAND_LINE_INPUT_BUFFER_SIZE (2048)
#define COMAMND_LINE_POOL_TAG (' dmC')

static char* CommandLineInputBuffer;
static u32 CommandLineInputWriteIndex;

KError_t InitCommandLine(void)
{
    CommandLineInputBuffer = AllocatePoolWithTag(PoolFlagNonPaged, 1 * COMMAND_LINE_INPUT_BUFFER_SIZE, COMAMND_LINE_POOL_TAG);
    zeromem_stosb(CommandLineInputBuffer, sizeof(*CommandLineInputBuffer) * COMMAND_LINE_INPUT_BUFFER_SIZE);

    CommandLineInputWriteIndex = 0;

    ConWriteString("> ");
    ConSwapBuffers();

    return KeyboardRegisterKeyCallback(CommandLineKeyboardCallback, 100);
}

void CommandLineRegisterCommand(const char* const name, const CommandLineCommand_f commandFunc)
{

}

static void CommandLineSubmit(void)
{
    const char* const dumpMemCmd = "dump-mem";
    const char* const echoCmd = "echo";

    ConNewLine();

    // Run Command

    u32 nameLength;

    for(nameLength = 0; nameLength < CommandLineInputWriteIndex && CommandLineInputBuffer[nameLength] && CommandLineInputBuffer[nameLength] != ' '; ++nameLength);

    if(StrCmpLen(CommandLineInputBuffer, nameLength, dumpMemCmd, StrLen(dumpMemCmd)) == 0)
    {
        DumpMemoryLayout();
    }
    else if(StrCmpLen(CommandLineInputBuffer, nameLength, echoCmd, StrLen(echoCmd)) == 0)
    {
        ConWriteString(CommandLineInputBuffer + nameLength + 1);
        ConNewLine();
    }

    CommandLineInputWriteIndex = 0;
    CommandLineInputBuffer[0] = '\0';

    ConWriteString("> ");
    ConSwapBuffers();
}

static void CommandLineAddCharacter(const u32 character)
{
    if(CommandLineInputWriteIndex == (COMMAND_LINE_INPUT_BUFFER_SIZE) - 1)
    {
        return;
    }

    CommandLineInputBuffer[CommandLineInputWriteIndex++] = (char) character;
    CommandLineInputBuffer[CommandLineInputWriteIndex] = '\0';

    ConWriteChar((char) character);
    ConSwapBuffers();
}

static void CommandLineBackspace(void)
{
    if(CommandLineInputWriteIndex == 0)
    {
        return;
    }

    CommandLineInputBuffer[--CommandLineInputWriteIndex] = '\0';

    ConBackspace();
    ConSwapBuffers();
}

static b8 CommandLineKeyboardCallback(PhysicalDeviceObject* const pdo, const u32 keyCode, const KeyboardToggleBits toggleBits)
{
    (void) toggleBits;

    if(KEY_IS_PRESSED(keyCode))
    {
        if(KEY_CODE(keyCode) == KEY_CODE_BACKSPACE)
        {
            // ConBackspace();
            CommandLineBackspace();
        }
        else if(KEY_CODE(keyCode) == KEY_CODE_ENTER)
        {
            // ConNewLine();
            CommandLineSubmit();
        }
        else
        {
            const u32 unicode = CodepointFromKeyCode(pdo, keyCode, KEYBOARD_LAYOUT_US_QWERTY);
            if(unicode)
            {
                CommandLineAddCharacter(unicode);
                // ConWriteChar(unicode);
            }
        }
    }

    return true;
}
