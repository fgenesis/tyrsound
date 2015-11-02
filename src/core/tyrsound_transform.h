#ifndef TYRSOUND_TRANSFORM_H
#define TYRSOUND_TRANSFORM_H

#include "tyrsound_ex.h"

#include "tyrsound_begin.h"

template<typename T> FORCE_INLINE T vmin(T a, T b) { return a < b ? a : b; }
template<typename T> FORCE_INLINE T vmax(T a, T b) { return a > b ? a : b; }
template<typename T> FORCE_INLINE T vclamp(T val, T lower, T upper) { return vmin(upper, vmax(lower, val)); }
template<typename T> FORCE_INLINE int vtoi(T x) { return static_cast<int>(x); }
template<typename T> FORCE_INLINE float vtof(T x) { return static_cast<float>(x); }

// identities
template<> FORCE_INLINE int vtoi<int>(int x) { return x; }
template<> FORCE_INLINE int vtoi<unsigned>(unsigned x) { return static_cast<int>(x); }
template<> FORCE_INLINE float vtof(float x) { return x; }

#include "tyrsound_end.h"


#if defined(_M_IX86) || defined(_M_AMD64)
#  include "tyrsound_intrin_sse.h"
#else
#  include "tyrsound_intrin_fallback.h"
#endif

#include "tyrsound_begin.h"

inline void *BufferAlloc(size_t sz)
{
    return AlignedAlloc(sz, BUFFER_ALIGNMENT);
}

// ---------------- Functions ---------------------

template<class OutputPointer, class InputPointer, class Operation>
struct _transform_aligned_blk
{
    template<unsigned BLOCK_SIZE>
    struct impl
    {
        static void apply(OutputPointer dst, InputPointer src, size_t length)
        {
            Operation op;
            tyrsound_compile_assert(BLOCK_SIZE > 1 && IsPowerOf2<BLOCK_SIZE>::value);
            const size_t endblock = length & ~(op.BLOCK_SIZE-1);
            size_t i = 0;
            for( ; i < endblock; i += op.BLOCK_SIZE)
                op.block(dst + i, src + i);
            for( ; i < length; ++i)
                op(dst + i, src + i);
        }
    };

    template<>
    struct impl<0>
    {
        static void apply(OutputPointer dst, InputPointer src, size_t length)
        {
            Operation op;
            for(size_t i = 0; i < length; ++i)
                op(dst + i, src + i);
        }
    };

    template<>
    struct impl<1>
    {
        static void apply(OutputPointer dst, InputPointer src, size_t length)
        {
            impl<0>::apply(dst, src, length);
        }
    };
};

template<class Operation, class OutputPointer, class InputPointer>
void transform_aligned(OutputPointer dst, InputPointer src, size_t length)
{
    _transform_aligned_blk<OutputPointer, InputPointer, Operation>::impl<Operation::BLOCK_SIZE>::apply(dst, src, length);
}


#include "tyrsound_end.h"

#endif

