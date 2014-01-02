#include <signal.h>
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

void breakpoint()
{
#if !TYRSOUND_IS_DEBUG
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


Mutex *Mutex::create()
{
    if(!tyrsound_ex_hasMT())
        return NULL;
    void *mtx = tyrsound_ex_newMutex();
    if(!mtx)
        return NULL;
    void *mem = Alloc(sizeof(Mutex));
    if(!mem)
        return NULL;
    return new(mem) Mutex(mtx);
}

void Mutex::destroy()
{
    this->~Mutex();
    Free(this);
}

Mutex::Mutex() : _on(tyrsound_ex_hasMT() != 0), _mtx(NULL)
{
    _init();
}

Mutex::Mutex(bool on) : _on(on), _mtx(NULL)
{
    _init();
}

Mutex::Mutex(void *mtx) : _on(true), _mtx(mtx)
{
}

void Mutex::_init()
{
    if(_on)
    {
        _mtx = tyrsound_ex_newMutex();
        if(!_mtx)
            breakpoint();
    }
}

Mutex::~Mutex()
{
    if(_on)
        tyrsound_ex_deleteMutex(_mtx);
}

bool Mutex::lock()
{
    return _on ? tyrsound_ex_lockMutex(_mtx) != 0 : true;
}

void Mutex::unlock()
{
    if(_on)
        tyrsound_ex_unlockMutex(_mtx);
}

#include "tyrsound_end.h"
