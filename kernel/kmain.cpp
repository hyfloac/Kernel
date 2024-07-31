
typedef int kprintf_f(const char* format, ...);

struct KernelFunctions
{
    kprintf_f* kprintf;
};

extern "C" int kmain(const KernelFunctions* const functions) noexcept
{
    functions->kprintf("Test print from ELF.\n");
    return 0;
}
