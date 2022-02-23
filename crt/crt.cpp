#ifdef _MSVC_LANG
int __cdecl _purecall()
{ }
#else
extern "C" void __cxa_pure_virtual()
{ }
#endif
