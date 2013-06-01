#include <signal.h>
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

void breakpoint()
{
#if TYRSOUND_IS_DEBUG
    return;
#endif
#ifdef _MSC_VER
#  ifdef _CrtDbgBreak
    _CrtDbgBreak();
#  else
    __debugbreak();
#  endif
#elif defined(__GNUC__) && ((__i386__) || (__x86_64__))
    __asm__ __volatile__ ( "int $3\n\t" );
#else
    signal(SIGTRAP);
#endif
}

#include "tyrsound_end.h"
