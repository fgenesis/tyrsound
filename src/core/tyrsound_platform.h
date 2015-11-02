#ifndef TYRSOUND_PLATFORM_H
#define TYRSOUND_PLATFORM_H


#if defined(_MSC_VER)
# include <malloc.h>
# define TYRSOUND_STACK_ALLOC(x) alloca(x)
# define TYRSOUND_STACK_FREE(x) /* nop */
#  if (_MSC_VER >= 1200)
#    define FORCE_INLINE __forceinline
#  else
#    define FORCE_INLINE __inline
#  endif
#elif defined(__GNUC__) || defined(__clang__)
# include <stdlib.h>
# define TYRSOUND_STACK_ALLOC(x) alloca(x)
# define TYRSOUND_STACK_FREE(x) /* nop */
#  ifdef NDEBUG
#    define FORCE_INLINE inline __attribute__((always_inline))
#  endif
#endif

#ifndef FORCE_INLINE
#  define FORCE_INLINE inline
#endif

#ifndef TYRSOUND_STACK_ALLOC
# define TYRSOUND_STACK_ALLOC(x) tyrsound_ex_alloc(NULL, x)
# define TYRSOUND_STACK_FREE(x) tyrsound_ex_alloc(x, 0)
#endif


// from SDL headers
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MISPEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define IS_BIG_ENDIAN 1
#endif

#ifndef IS_BIG_ENDIAN
#  define IS_BIG_ENDIAN 0
#endif


#include "tyrsound_begin.h"

// sample types
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;



#include "tyrsound_end.h"


#endif
