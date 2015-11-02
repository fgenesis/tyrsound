#ifndef TYRSOUND_UTIL_H
#define TYRSOUND_UTIL_H

#include "tyrsound_platform.h"

#include "tyrsound_begin.h"

// ---------------- Helpers -------------------------

template<class T, class U>
struct is_same {
    enum { value = 0 };
};

template<class T>
struct is_same<T, T> {
    enum { value = 1 };
};

template<unsigned N>
class IsPowerOf2
{
public:
    enum { value = N && !(N & (N - 1)) };
};

template<unsigned N>
class NextPowerOf2
{
private:
    enum
    {
        _1 = N - 1u,
        _2 = _1 | (_1 >> 1u),
        _3 = _2 | (_2 >> 2u),
        _4 = _3 | (_3 >> 4u),
        _5 = _4 | (_4 >> 8u),
        _6 = _5 | (_5 >> 16u),
        _7 = _6 + 1u
    };
public:
    enum { value = _7 + (_7 == 0) };
};

// --------------- Endian --------------------------

namespace Endian
{
    template<int T> FORCE_INLINE void _convert(char *val)
    {
        char *p = val + T - 1;
        char t = *p;
        *p = *val;
        *val = t;
        _convert<T - 2>(val + 1);
    }
    template<> FORCE_INLINE void _convert<0>(char *) {}
    template<> FORCE_INLINE void _convert<1>(char *) {}

    template<typename T> FORCE_INLINE void _Convert(T *val)
    {
        _convert<sizeof(T)>((char *)(val));
    }

    template<typename T> FORCE_INLINE void Convert(T& val)
    {
        _Convert(&val);
    }

#if IS_BIG_ENDIAN
    template<typename T> FORCE_INLINE void ToLittleEndian(T& val) { Convert<T>(&val); }
    template<typename T> FORCE_INLINE void ToBigEndian(T&) { }
#else
    template<typename T> FORCE_INLINE void ToLittleEndian(T&) { }
    template<typename T> FORCE_INLINE void ToBigEndian(T& val) { Convert<T>(&val); }
#endif

    template<typename T> void ToLittleEndian(T*);   // will generate link error
    template<typename T> void ToBigEndian(T*);      // will generate link error

#if defined(_MSC_VER)
    template<> FORCE_INLINE void Convert<s16>(s16& val) { val = _byteswap_ushort(val); }
    template<> FORCE_INLINE void Convert<u16>(u16& val) { val = _byteswap_ushort(val); }
    template<> FORCE_INLINE void Convert<s32>(s32& val) { val = _byteswap_ulong(val); }
    template<> FORCE_INLINE void Convert<u32>(u32& val) { val = _byteswap_ulong(val); }
#endif
};

#include "tyrsound_end.h"

#endif
