
/* This file exposes API for extensions.
 * It does not need to be included if you just want to use the library.
 * If you plan to add decoders or output devices, include it where necessary.
 * Note that this is a C++ header.
*/

#ifndef TYRSOUND_EX_H
#define TYRSOUND_EX_H

#include "tyrsound.h"

#include "tyrsound_begin.h"

#if !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
#  define TYRSOUND_IS_DEBUG 1
#endif

struct DeviceInfo;
class DecoderBase;
class DecoderFactoryBase;
class ChannelBase;
class DeviceBase;
class DeviceFactoryBase;

// Functions provided by the library, for use by extensions
extern "C"
{
    TYRSOUND_DLL_EXPORT void *tyrsound_ex_alloc(void *ptr, size_t size);

    TYRSOUND_DLL_EXPORT int tyrsound_ex_hasMT(); // multithreading available & set up?
    TYRSOUND_DLL_EXPORT void *tyrsound_ex_newMutex();
    TYRSOUND_DLL_EXPORT void tyrsound_ex_deleteMutex(void *);
    TYRSOUND_DLL_EXPORT int tyrsound_ex_lockMutex(void *);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_unlockMutex(void *);

    TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDevice(const DeviceInfo&);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDecoder(DecoderFactoryBase *);

    TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadLibrary(const char *name);
    TYRSOUND_DLL_EXPORT void tyrsound_ex_unloadLibrary(void *);
    TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadFunction(void *, const char *name);

    TYRSOUND_DLL_EXPORT void tyrsound_ex_message(tyrsound_MessageSeverity, const char *);
}

// Memory related functions, using the allocator set via tyrsound_setAlloc()
inline void *Realloc(void *ptr, size_t size) { return tyrsound_ex_alloc(ptr, size); }
inline void  Free(void *ptr)                 { tyrsound_ex_alloc(ptr, 0); }
inline void *Alloc(size_t size)              { return tyrsound_ex_alloc(NULL, size); }


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
