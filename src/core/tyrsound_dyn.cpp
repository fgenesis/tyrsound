#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else // Assume posix
#  include <dlfcn.h>
#endif

void *dynopen(const char *fn)
{
    void *h = NULL;
    size_t len = strlen(fn);
    char *s = (char*)Alloc(len + 10);
    memcpy(s, fn, len);
    s[len] = 0;
#ifdef _WIN32
    memcpy(s + len, ".dll\0", 5);
    h =  LoadLibrary(s);
#else
    memcpy(s + len, ".so\0", 4);
    h = *dlopen(s, RTLD_NOW | RTLD_LOCAL);
#endif
    Free(s);
    return h;
}

void dynclose(void *h)
{
    if(h)
    {
#ifdef _WIN32
        FreeLibrary((HMODULE)h);
#else
        dlclose(h);
#endif
    }
}

void *dynsym(void *h, const char *name)
{
    void *func = NULL;
#ifdef _WIN32
    func = GetProcAddress((HMODULE)h, name);
#else
    func = dlsym(h, name);
#endif
    return func;
}


#include "tyrsound_end.h"

