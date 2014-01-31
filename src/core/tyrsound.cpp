#include <cstdarg>
#include <cstdio>

#include "tyrsound.h"
#include "tyrsound_internal.h"
#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

// Global state
static tyrsound_Alloc s_alloc = NULL;
static void *s_alloc_user = NULL;

static void *(*s_newMutexFunc)(void) = NULL;
static void (*s_deleteMutexFunc)(void*)= NULL;
static int (*s_lockMutexFunc)(void*) = NULL;
static void (*s_unlockMutexFunc)(void*) = NULL;

static void *s_msgPtr = NULL;
#if !TYRSOUND_IS_DEBUG
static tyrsound_MessageCallback s_msgCallback = NULL;
#else
static void _debugMsg(tyrsound_MessageSeverity severity, const char *str, void *user)
{
    printf("[%d] %s\n", severity, str);
}
static tyrsound_MessageCallback s_msgCallback = _debugMsg;
#endif


void *tyrsound_ex_alloc(void *ptr, size_t size)
{
    if(s_alloc)
        return s_alloc(ptr, size, s_alloc_user);

    return realloc(ptr, size);
}

int tyrsound_ex_hasMT()
{
    return s_newMutexFunc && s_deleteMutexFunc && s_lockMutexFunc && s_unlockMutexFunc;
}

void *tyrsound_ex_newMutex()
{
    return s_newMutexFunc ? s_newMutexFunc() : NULL;
}

void tyrsound_ex_deleteMutex(void *mtx)
{
    if(s_deleteMutexFunc)
        s_deleteMutexFunc(mtx);
}

TYRSOUND_DLL_EXPORT int tyrsound_ex_lockMutex(void *mtx)
{
    return s_lockMutexFunc ? s_lockMutexFunc(mtx) : (mtx ? TYRSOUND_ERR_NOT_READY : TYRSOUND_ERR_INVALID_HANDLE);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_unlockMutex(void *mtx)
{
    if(s_unlockMutexFunc)
        s_unlockMutexFunc(mtx);
}

TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadLibrary(const char *name)
{
    return dynopen(name);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_unloadLibrary(void *h)
{
    return dynclose(h);
}

TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadFunction(void *h, const char *name)
{
    return dynsym(h, name);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_message(tyrsound_MessageSeverity severity, const char *str)
{
    if(s_msgCallback)
        s_msgCallback(severity, str, s_msgPtr);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_messagef(tyrsound_MessageSeverity severity, const char *fmt, ...)
{
    if(s_msgCallback)
    {
        const int BUFSIZE = 1024;
        char buf[BUFSIZE];
        va_list va;
        va_start(va, fmt);
#ifdef _MSC_VER
        vsnprintf_s(&buf[0], BUFSIZE-1, _TRUNCATE, fmt, va);
#else
        vsnprintf(&buf[0], BUFSIZE-1, fmt, va);
#endif
        va_end(va);
        s_msgCallback(severity, buf, s_msgPtr);
    }
}

#include "tyrsound_end.h"



tyrsound_Error tyrsound_init(const tyrsound_Format *fmt, const char *output)
{
    if(tyrsound::getDevice())
        return TYRSOUND_ERR_UNSPECIFIED;

    bool haveDevice = false;

    if(!output || !*output)
    {
        haveDevice = tyrsound::initDevice(NULL, fmt);
    }
    else
    {
        char buf[32];
        const char *next, *prev = output;
        while(true)
        {
            next = strchr(prev, ' ');
            unsigned int len = tyrsound::Min<unsigned int>(next - prev, sizeof(buf) - 2);
            memcpy(buf, prev, len);
            buf[len+1] = 0;
            prev = next + 1;

            if(tyrsound::initDevice(&buf[0], fmt))
            {
                haveDevice = true;
                break;
            }

            if(!next)
                break;
        }
    }

    if(!haveDevice)
        return TYRSOUND_ERR_NO_DEVICE;

    tyrsound::initDecoders();

    return tyrsound::initSounds();
}

tyrsound_Error tyrsound_shutdown()
{
    tyrsound::shutdownSounds();
    tyrsound::shutdownDevice();
    tyrsound::shutdownDecoders();
    tyrsound_setAlloc(NULL, NULL);
    return TYRSOUND_ERR_OK;
}

tyrsound_Error tyrsound_setupMT(void *(*newMutexFunc)(void), void (*deleteMutexFunc)(void*), int (*lockFunc)(void*), void (*unlockFunc)(void*))
{
    tyrsound::s_newMutexFunc = newMutexFunc;
    tyrsound::s_deleteMutexFunc = deleteMutexFunc;
    tyrsound::s_lockMutexFunc = lockFunc;
    tyrsound::s_unlockMutexFunc = unlockFunc;

    return TYRSOUND_ERR_OK;
}

void tyrsound_setAlloc(tyrsound_Alloc allocFunc, void *user)
{
    tyrsound::s_alloc = allocFunc;
    tyrsound::s_alloc_user = user;
}



tyrsound_Error tyrsound_update(void)
{
    tyrsound::DeviceBase *device = tyrsound::getDevice();
    if(!device)
        return TYRSOUND_ERR_NO_DEVICE;

    device->update();
    tyrsound_Error err = tyrsound::updateSounds();
    return err;
}

