#ifndef TYRSOUND_INTRIN_SSE_H
#define TYRSOUND_INTRIN_SSE_H

#include <intrin.h>
#include <xmmintrin.h>
#include "tyrsound_platform.h"

#include "tyrsound_begin.h"

enum
{
    BUFFER_ALIGNMENT = 16
};


template<> FORCE_INLINE float vmin<float>(float a, float b)
{
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
template<> FORCE_INLINE float vmax<float>(float a, float b)
{
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
template<> FORCE_INLINE float vclamp<float>(float val, float lower, float upper)
{
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(upper), _mm_max_ss(_mm_set_ss(lower), _mm_set_ss(val))));
}
template<> FORCE_INLINE int vtoi<float>(float x)
{
    return _mm_cvtt_ss2si(_mm_set_ss(x));
}
FORCE_INLINE float itof(int x)
{
    return _mm_cvtss_f32(_mm_cvt_si2ss(_mm_setzero_ps(), x));
}

struct CvtFloatToInt
{
    enum { BLOCK_SIZE = 8 };
    FORCE_INLINE void operator()(int *dst, const float *src)
    {
        *dst = vtoi(*src);
    }
    FORCE_INLINE void block(int *dst, const float *src)
    {
        union
        {
            __m128i *pi;
            int *i;
        } d;
        d.i = dst;
        __m128 f1 = _mm_load_ps(src);
        __m128 f2 = _mm_load_ps(src + 4);
        __m128i i1 = _mm_cvtps_epi32(f1);
        __m128i i2 = _mm_cvtps_epi32(f2);
        _mm_store_si128(d.pi, i1);
        _mm_store_si128(d.pi + 1, i2);
    }
};

struct CvtIntToFloat
{
    enum { BLOCK_SIZE = 8 };
    FORCE_INLINE void operator()(float *dst, const int *src)
    {
        *dst = itof(*src);
    }
    FORCE_INLINE void block(float *dst, const int *src)
    {
        union
        {
            __m128i *pi;
            const int *i;
        } s;
        s.i = src;
        __m128i i1 = _mm_load_si128(s.pi);
        __m128i i2 = _mm_load_si128(s.pi + 1);
        __m128 f1 = _mm_cvtepi32_ps(i1);
        __m128 f2 = _mm_cvtepi32_ps(i2);
        _mm_store_ps(dst, f1);
        _mm_store_ps(dst + 4, f2);
    }
};


struct CvtSampleFloatToS16
{
    enum { BLOCK_SIZE = 8 };
    CvtSampleFloatToS16()
        : scaler(_mm_set_ps1(32768.0f)), maxval(_mm_set_ps1(1.0f)), minval(_mm_set_ps1(-1.0f))
    {
    }
    FORCE_INLINE void operator()(s16 *dst, const float *src)
    {
        *dst = vtoi(*src);
    }
    FORCE_INLINE void block(s16 *dst, const float *src)
    {
        union
        {
            __m128i *pi;
            s16 *i;
        } d;
        d.i = dst;
        __m128 f1 = _mm_load_ps(src);
        __m128 f2 = _mm_load_ps(src + 4);
        f1 = _mm_min_ps(_mm_max_ps(f1, minval), maxval);
        f2 = _mm_min_ps(_mm_max_ps(f2, minval), maxval);
        f1 = _mm_mul_ps(f1, scaler);
        f2 = _mm_mul_ps(f2, scaler);
        __m128i i1 = _mm_cvtps_epi32(f1);
        __m128i i2 = _mm_cvtps_epi32(f2);
        __m128i ps = _mm_packs_epi32(i1, i2);
        _mm_store_si128(d.pi, ps);
    }
private:
    const __m128 scaler;
    const __m128 maxval;
    const __m128 minval;
};




#include "tyrsound_end.h"

#endif

