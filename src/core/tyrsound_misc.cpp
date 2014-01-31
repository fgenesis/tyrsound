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

bool isBigEndian()
{
    union { int i; char c; } endian;
    endian.i = 1;
    return !endian.c;
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

unsigned int readLE32(const void *buf)
{
    unsigned char *x = (unsigned char*)buf;
    return x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
}

unsigned short readLE16(const void *buf)
{
    unsigned char *x = (unsigned char*)buf;
    return x[0] | (x[1] << 8);
}

void writeLE32(void *buf, unsigned int i)
{
    unsigned char *x = (unsigned char*)buf;
    x[0] = i >> 0;
    x[1] = i >> 8;
    x[2] = i >> 16;
    x[3] = i >> 24;
}

void writeLE16(void *buf, unsigned short i)
{
    unsigned char *x = (unsigned char*)buf;
    x[0] = i >> 0;
    x[1] = i >> 8;
}

#include "tyrsound_end.h"
