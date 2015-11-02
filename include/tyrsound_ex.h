
/* This file exposes API for extensions.
 * It does not need to be included if you just want to use the library.
 * If you plan to add decoders or output devices, include it where necessary.
 * Note that this is a C++ header.
*/

#ifndef TYRSOUND_EX_H
#define TYRSOUND_EX_H

#include "tyrsound.h"


#define tyrsound_compile_assert(cond) switch(0){case 0:; case cond:;}

#if !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
#  define TYRSOUND_IS_DEBUG 1
#endif

#include "tyrsound_begin.h"
struct DeviceInfo;
class DecoderBase;
class DecoderFactoryBase;
class ChannelBase;
class DeviceBase;
class DeviceFactoryBase;
#include "tyrsound_end.h"

// Functions provided by the library, for use by extensions
extern "C"
{
    TYRSOUND_DLL_EXPORT void *tyrsound_ex_alloc(void *ptr, size_t size);

    TYRSOUND_DLL_EXPORT int tyrsound_ex_hasMT(); // multithreading available & set up?
    TYRSOUND_DLL_EXPORT void *tyrsound_ex_newMutex();
    TYRSOUND_DLL_EXPORT void tyrsound_ex_deleteMutex(void *);
    TYRSOUND_DLL_EXPORT int tyrsound_ex_lockMutex(void *);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_unlockMutex(void *);

    TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDevice(const tyrsound::DeviceInfo&);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDecoder(tyrsound::DecoderFactoryBase *);

    TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadLibrary(const char *name);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_unloadLibrary(void *);
    TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadFunction(void *, const char *name);

    TYRSOUND_DLL_EXPORT void tyrsound_ex_message(tyrsound_MessageSeverity, const char *);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_messagef(tyrsound_MessageSeverity, const char *, ...);
}

#include "tyrsound_begin.h"

inline void *alignPointer(void *ptr, size_t align)
{
    tyrsound_compile_assert(sizeof(uintptr_t) == sizeof(void*));
    if(!align)
        return ptr;
    --align;
    uintptr_t p = uintptr_t(ptr);
    return (void*)((p + align) & ~uintptr_t(align));
}
inline bool pointerIsAligned(void *ptr, size_t align)
{
    return !align || !(uintptr_t(ptr) % uintptr_t(align));
}

// Memory related functions, using the allocator set via tyrsound_setAlloc()
inline void *Realloc(void *ptr, size_t size) { return tyrsound_ex_alloc(ptr, size); }
inline void  Free(void *ptr)                 { tyrsound_ex_alloc(ptr, 0); }
inline void *Alloc(size_t size)              { return tyrsound_ex_alloc(NULL, size); }
inline void *AlignedAlloc(size_t size, size_t alignment)
{
    unsigned char *p = (unsigned char*)Alloc(size+alignment); // in theory, up to (alignment-1) bytes are needed as padding. one byte extra is for the offset
    if(!p)
        return NULL;
    unsigned char *a = (unsigned char*)alignPointer(p+1, alignment); // always make sure at least 1 byte is free, that will hold the offset
    a[-1] = (unsigned char)(a - p);
    return a;
}
inline void AlignedFree(void *ptr)
{
    if(!ptr)
        return;
    unsigned char *a = (unsigned char*)ptr;
    unsigned off = a[-1]; // always >= 1
    void *p = a - off;
    Free(p);
}


// Body is in tyrsound_misc.cpp
class Mutex
{
public:
    static Mutex *create();
    void destroy();
    Mutex();
    ~Mutex();
    bool lock();
    void unlock();
private:
    void *_mtx;
    const bool _on;
    // Internally used
    Mutex(bool);
    Mutex(void *);
    void _init();
};

class MutexGuard
{
public:
    MutexGuard(Mutex& m) : _m(&m), _ok(m.lock()) {}
    MutexGuard(Mutex *m) : _m(m), _ok(!m || m->lock()) {}
    ~MutexGuard() { if(_m) _m->unlock(); }
    operator int() const { return _ok; }
    void unlock() { if(_m) { _m->unlock(); _m = NULL; _ok = false; } }
private:
    int _ok;
    Mutex *_m;
};

template<typename T>       T& Min(      T& a,       T& b) { return a < b ? a : b; }
template<typename T> const T& Min(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>       T& Max(      T& a,       T& b) { return a > b ? a : b; }
template<typename T> const T& Max(const T& a, const T& b) { return a > b ? a : b; }

#define TYRSOUND_STATIC_REGISTER(registrar, type, expr) \
    tyrsound:: registrar <type> _static_autoregister_##registrar##_##type expr ; \
    extern "C" TYRSOUND_DLL_EXPORT void *_static_autoregister_helper_##registrar##_##type() \
    { return &_static_autoregister_##registrar##_##type; }


#include "tyrsound_end.h"

#endif
