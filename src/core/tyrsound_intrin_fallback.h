#ifndef TYRSOUND_INTRIN_FALLBACK_H
#define TYRSOUND_INTRIN_FALLBACK_H

#include "tyrsound_begin.h"

enum
{
    BUFFER_ALIGNMENT = 4
};

FORCE_INLINE float itof(int x) { return static_cast<float>(x); }

struct CvtFloatToInt
{
    enum { BLOCK_SIZE = 0 };
    FORCE_INLINE void operator()(int *dst, const float *src)
    {
        *dst = vtoi(*src);
    }
};

struct CvtIntToFloat
{
    enum { BLOCK_SIZE = 0 };
    FORCE_INLINE void operator()(float *dst, const int *src)
    {
        *dst = itof(*src);
    }
};



#include "tyrsound_end.h"

#endif

