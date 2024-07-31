#include <kstdint.h>
#include <stddef.h>

void* malloc(size_t size)
{
    (void) size;
    return nullptr;
}

void free(void* p)
{
    (void) p;
}

void *operator new(size_t size)
{
    return malloc(size);
}
 
void *operator new[](size_t size)
{
    return malloc(size);
}
 
void operator delete(void *p)
{
    free(p);
}
 
void operator delete[](void *p)
{
    free(p);
}
 
void operator delete(void *p, long unsigned int)
{
    free(p);
}
 
void operator delete[](void *p, long unsigned int)
{
    free(p);
}
