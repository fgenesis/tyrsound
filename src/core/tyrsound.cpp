#include <cstdarg>
#include <cstdio>

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
    if(severity >= TYRSOUND_MSG_DEBUG)
        printf("tyrsound[%d]: %s\n", severity, str);
}
static tyrsound_MessageCallback s_msgCallback = _debugMsg;
#endif

#include "tyrsound_end.h"

extern "C" {

TYRSOUND_DLL_EXPORT void *tyrsound_ex_alloc(void *ptr, size_t size)
{
    if(tyrsound::s_alloc)
        ptr = tyrsound::s_alloc(ptr, size, tyrsound::s_alloc_user);
    else
    {
        // From the C99 standard. regarding malloc(0):
        // If the size of the space requested is zero, the behavior is implementation- deﬁned [...]
        // Apparently some use malloc(0) for realloc(p, 0), so we should not just assume realloc(p, 0) was *equal* to free()
        //ptr = realloc(ptr, size);
        if(!ptr && size)
            ptr = malloc(size);
        else if(ptr && !size)
        {
            free(ptr);
            ptr = NULL;
        }
        else if(ptr && size)
            ptr = realloc(ptr, size);
    }

    if(!ptr && size)
    {
        // Intentionally send 2 messages, in case the formatting tries to alloc
        tyrsound_ex_message (TYRSOUND_MSG_ERROR, "Out of memory?");
        tyrsound_ex_messagef(TYRSOUND_MSG_ERROR, "  Allocation of %u bytes failed!", (unsigned)size);
    }

    return ptr;
}

TYRSOUND_DLL_EXPORT int tyrsound_ex_hasMT()
{
    return tyrsound::s_newMutexFunc && tyrsound::s_deleteMutexFunc && tyrsound::s_lockMutexFunc && tyrsound::s_unlockMutexFunc;
}

TYRSOUND_DLL_EXPORT void *tyrsound_ex_newMutex()
{
    return tyrsound::s_newMutexFunc ? tyrsound::s_newMutexFunc() : NULL;
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_deleteMutex(void *mtx)
{
    if(tyrsound::s_deleteMutexFunc)
        tyrsound::s_deleteMutexFunc(mtx);
}

TYRSOUND_DLL_EXPORT int tyrsound_ex_lockMutex(void *mtx)
{
    return tyrsound::s_lockMutexFunc ? tyrsound::s_lockMutexFunc(mtx) : (mtx ? TYRSOUND_ERR_NOT_READY : TYRSOUND_ERR_INVALID_HANDLE);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_unlockMutex(void *mtx)
{
    if(tyrsound::s_unlockMutexFunc)
        tyrsound::s_unlockMutexFunc(mtx);
}

TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadLibrary(const char *name)
{
    void *h = NULL;
    if((h = tyrsound::dynopen(name)))
        return h;

    size_t len = strlen(name);
    char *buf = (char*)alloca(len + 10);
    memcpy(buf, "lib", 3);
    memcpy(buf+3, name, len+1);

    if((h = tyrsound::dynopen(buf)))
        return h;

    memcpy(buf+3+len, "-0", 3);
    if((h = tyrsound::dynopen(buf)))
        return h;

    return tyrsound::dynopen(buf+3);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_unloadLibrary(void *h)
{
    return tyrsound::dynclose(h);
}

TYRSOUND_DLL_EXPORT void *tyrsound_ex_loadFunction(void *h, const char *name)
{
    return tyrsound::dynsym(h, name);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_message(tyrsound_MessageSeverity severity, const char *str)
{
    if(tyrsound::s_msgCallback)
        tyrsound::s_msgCallback(severity, str, tyrsound::s_msgPtr);
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_messagef(tyrsound_MessageSeverity severity, const char *fmt, ...)
{
    if(tyrsound::s_msgCallback)
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
        tyrsound::s_msgCallback(severity, buf, tyrsound::s_msgPtr);
    }
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_init(const tyrsound_Format *fmt, const tyrsound_DeviceConfig *cfg)
{
    if(tyrsound::getDevice())
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Already initialized");
        return TYRSOUND_ERR_UNSPECIFIED;
    }

#if TYRSOUND_IS_DEBUG
    tyrsound_ex_message(TYRSOUND_MSG_INFO, "This is a debug build");
#endif

    bool haveDevice = false;
    const char *output = cfg ? cfg->deviceName : NULL;

    if(!output || !*output)
    {
        haveDevice = tyrsound::initDevice(NULL, fmt, cfg);
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
            if(tyrsound::initDevice(&buf[0], fmt, cfg))
            {
                haveDevice = true;
                break;
            }

            if(!next)
                break;
        }
    }

    if(!haveDevice)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "No usable device");
        return TYRSOUND_ERR_NO_DEVICE;
    }

    tyrsound::initDecoders();
    tyrsound::initGroups();

    return tyrsound::initSounds();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_shutdown()
{
    tyrsound::shutdownSounds();
    tyrsound::shutdownGroups();
    tyrsound::shutdownDevice();
    tyrsound::shutdownDecoders();
    tyrsound_ex_message(TYRSOUND_MSG_INFO, "Shutdown complete");
    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setupMT(void *(*newMutexFunc)(void), void (*deleteMutexFunc)(void*), int (*lockFunc)(void*), void (*unlockFunc)(void*))
{
    tyrsound::s_newMutexFunc = newMutexFunc;
    tyrsound::s_deleteMutexFunc = deleteMutexFunc;
    tyrsound::s_lockMutexFunc = lockFunc;
    tyrsound::s_unlockMutexFunc = unlockFunc;

    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT void tyrsound_setAlloc(tyrsound_Alloc allocFunc, void *user)
{
    tyrsound::s_alloc = allocFunc;
    tyrsound::s_alloc_user = user;
}



TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_update(void)
{
    tyrsound::DeviceBase *device = tyrsound::getDevice();
    if(!device)
        return TYRSOUND_ERR_NO_DEVICE;

    device->update();
    tyrsound_Error err = tyrsound::updateSounds();
    return err;
}

} // end extern "C"
