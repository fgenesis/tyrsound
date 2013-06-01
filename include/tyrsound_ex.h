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

extern "C"
{
	TYRSOUND_DLL_EXPORT void *tyrsound_ex_alloc(void *ptr, size_t size);
	TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDevice(const DeviceInfo&);
	TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDecoder(DecoderFactoryBase *);
}

inline void *Realloc(void *ptr, size_t size) { return tyrsound_ex_alloc(ptr, size); }
inline void  Free(void *ptr)                 { tyrsound_ex_alloc(ptr, 0); }
inline void *Alloc(size_t size)              { return tyrsound_ex_alloc(NULL, size); }

template<typename T>       T& Min(      T& a,       T& b) { return a < b ? a : b; }
template<typename T> const T& Min(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>       T& Max(      T& a,       T& b) { return a > b ? a : b; }
template<typename T> const T& Max(const T& a, const T& b) { return a > b ? a : b; }

#define TYRSOUND_STATIC_REGISTER(registrar, type, expr) \
    registrar<type> _static_autoregister_##registrar##_##type expr ; \
    extern "C" TYRSOUND_DLL_EXPORT void *_static_autoregister_helper_##registrar##_##type() \
    { return &_static_autoregister_##registrar##_##type; }


#include "tyrsound_end.h"

#endif
