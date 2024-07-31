#pragma once

typedef int kprintf_f(const char* format, ...);

struct KernelFunctions
{
    kprintf_f* kprintf;
};

