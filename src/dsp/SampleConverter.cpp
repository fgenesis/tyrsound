#include <string.h> // for memcpy()
#include <assert.h>

#include "tyrsound_internal.h"
#include "tyrsound_transform.h"

#include "tyrsound_begin.h"


// TODO: make the functions here work with SSE blocks and not SSE scalar values only


// ---------------- Sign ------------------------------

template<typename T>
struct MakeSigned {};

#define IDENTSIGN(T) \
    template<> \
struct MakeSigned<T> \
{ \
    typedef T result_type; \
    FORCE_INLINE T operator() (T x) const \
{ \
    return x; \
} \
};

IDENTSIGN(s8)
IDENTSIGN(s16)
IDENTSIGN(f32)

template<>
struct MakeSigned<u8>
{
    typedef s8 result_type;
    FORCE_INLINE s8 operator() (u8 x) const
    {
        return static_cast<s8>(x ^ 0x80);
    }
};

template<>
struct MakeSigned<u16>
{
    typedef s16 result_type;
    FORCE_INLINE s16 operator() (u16 x) const
    {
        return static_cast<s16>(x ^ 0x8000);
    }
};


// ------------ Sample conversion ----------------------------

template<typename From, typename To>
struct Convert;

template<typename Same>
struct Convert<Same, Same>
{
    FORCE_INLINE Same operator() (Same x) const
    {
        return x;
    }
};

template<>
struct Convert<f32, s8>
{
    FORCE_INLINE s8 operator() (f32 x) const
    {
        return vtoi(vclamp(x, -1.0f, 1.0f) * 128.0f + 0.5f);
    }
};

template<>
struct Convert<float, s16>
{
    FORCE_INLINE s16 operator() (f32 x) const
    {
        return vtoi(vclamp(x, -1.0f, 1.0f) * 32768.0f + 0.5f);
    }
};

template<>
struct Convert<s8, f32>
{
    FORCE_INLINE f32 operator() (char x) const
    {
        return itof(x) * (1.0f / f32(1<<7));
    }
};

template<>
struct Convert<s16, f32>
{
    FORCE_INLINE f32 operator() (s16 x) const
    {
        return itof(x) * (1.0f / f32(1<<15));
    }
};

template<>
struct Convert<s8, s16>
{
    FORCE_INLINE s16 operator() (s8 x) const
    {
        return static_cast<s16>(x << 8);
    }
};

template<>
struct Convert<s16, s8>
{
    FORCE_INLINE s8 operator() (s16 x) const
    {
        return static_cast<s8>(x >> 8);
    }
};


template<typename Same>
FORCE_INLINE void convertCopy(Same *dst, const Same *src, size_t sz)
{
    memcpy(dst, src, sz * sizeof(Same));
}

template<typename From, typename To>
void convertCopy(To *dst, const From *src, size_t sz)
{
    MakeSigned<From> sign;
    Convert<MakeSigned<From>::result_type, To> cv;

    for(size_t i = 0; i < sz; ++i)
        dst[i] = cv(sign(src[i]));
}

// ---------------- Interleave ---------------------------

template<typename From, typename To>
void interleaveConvertCopy(To *dst, From *src, size_t sz, unsigned spacing)
{
    MakeSigned<From> sign;
    Convert<MakeSigned<From>::result_type, To> cv;

    for(unsigned block = 0; block < spacing; ++block)
        for(size_t j = block; j < sz; j += spacing)
            dst[j] = cv(sign(*src++));
}

template<typename T>
void interleaveCopy(T *dst, const T *src, size_t sz, unsigned spacing)
{
    for(unsigned block = 0; block < spacing; ++block)
        for(size_t j = block; j < sz; j += spacing)
            dst[j] = *src++;
}

// ---------------- Deinterleave ---------------------

template<typename From, typename To>
void deinterleaveConvertCopy (To *dst, From *src, size_t sz, unsigned spacing)
{
    MakeSigned<From> sign;
    Convert<MakeSigned<From>::result_type, To> cv;
    size_t blocksize = sz / spacing;
    for(unsigned block = 0; block < blocksize; ++block)
        for(size_t i = block; i < sz; i += blocksize)
            dst[i] = sign(cv(*src++));
}

template<typename T>
void deinterleaveCopy(T *dst, const T *src, size_t sz, unsigned spacing)
{
    size_t blocksize = sz / spacing;
    for(unsigned block = 0; block < blocksize; ++block)
        for(size_t i = block; i < sz; i += blocksize)
            dst[i] = *src++;
}

// ------------- Endian conversion ------------

template<typename T> void toNativeEndianInplace(tyrsound_Format &fmt, T *buf, size_t sz)
{
    if(!fmt.bigendian == !(IS_BIG_ENDIAN))
        return;

    for(size_t i = 0; i < sz; ++i)
        Endian::Convert<T>(buf[i]);
    fmt.bigendian = !!(IS_BIG_ENDIAN);
}


template <typename To>
void copyConvertT(tyrsound_Format &fmt, To *dst, void *src, size_t sz)
{
    if(fmt.isfloat)
    {
        switch(fmt.sampleBits)
        {
        case 32:
            toNativeEndianInplace<f32>(fmt, (f32*)src, sz);
            convertCopy<f32, To>(dst, (f32*)src, sz);
            break;

        default:
            breakpoint();
            assert(false && "fmt.sampleBits (float) is not 32");
        }
    }
    else
    {
        switch(fmt.sampleBits)
        {
        case 8:
            if(fmt.signedSamples)
                convertCopy<s8, To>(dst, (s8*)src, sz);
            else
                convertCopy<u8, To>(dst, (u8*)src, sz);
            break;

        case 16:
            toNativeEndianInplace<u16>(fmt, (u16*)src, sz);
            if(fmt.signedSamples)
                convertCopy<s16, To>(dst, (s16*)src, sz);
            else
                convertCopy<u16, To>(dst, (u16*)src, sz);
            break;

        default:
            breakpoint();
            assert(false && "fmt.sampleBits is not 8 or 16");
        }
    }
    fmt.bigendian = IS_BIG_ENDIAN;
    fmt.sampleBits = sizeof(To) * 8;
}

// ------------ X -> float -------------------

void copyConvertToFloat(tyrsound_Format &fmt, float *dst, void *src, size_t sz)
{
    copyConvertT(fmt, dst, src, sz);
    fmt.isfloat = 1;
    fmt.signedSamples = -1;
}

void copyConvertToS16(tyrsound_Format &fmt, s16 *dst, void *src, size_t sz)
{
    copyConvertT(fmt, dst, src, sz);
    fmt.isfloat = 0;
    fmt.signedSamples = 11;
}

void convertFloatToS16Inplace(tyrsound_Format& fmt, void *buf, size_t samples)
{
    assert(fmt.isfloat);

    const f32 *in = (f32*)buf;
    s16 *out = (s16*)buf;
    /*const f32 *end = in + samples;
    Convert<f32, s16> cv;
    for( ; in < end; ++in)
        *out++ = cv(*in);*/

    transform_aligned<CvtSampleFloatToS16>(out, in, samples);

    fmt.bigendian = IS_BIG_ENDIAN;
    fmt.isfloat = 0;
    fmt.sampleBits = 16;
    fmt.signedSamples = 1;
}


#include "tyrsound_end.h"
