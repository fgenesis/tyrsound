#include "tyrsound.h"
#include "tyrsound_internal.h"
#include "DeviceBase.h"

#include "tyrsound_begin.h"

// Global state
tyrsound_Alloc g_alloc = NULL;

static void *s_updateMutex = NULL;
static int (*s_lockMutexFunc)(void*) = NULL;
static void (*s_unlockMutexFunc)(void*) = NULL;

static const char *defaultOutputs = "openal";


#include "tyrsound_end.h"


int lockUpdate()
{
    return tyrsound::s_lockMutexFunc
        ? tyrsound::s_lockMutexFunc(tyrsound::s_updateMutex)
        : 1; // 1 if lock was successful or not necessary
}

void unlockUpdate()
{
    if(tyrsound::s_unlockMutexFunc)
        tyrsound::s_unlockMutexFunc(tyrsound::s_updateMutex);
}


static void *_defaultAlloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}


tyrsound_Error tyrsound_init(const tyrsound_Format *fmt, const char *output)
{
    tyrsound_setAlloc(tyrsound::g_alloc); // fixes NULL

    if(!output || !*output)
        output = tyrsound::defaultOutputs;

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
            goto output_done;

        if(!next)
            break;
    }

    return TYRSOUND_ERR_NO_DEVICE;

output_done:

    return TYRSOUND_ERR_OK;
}

tyrsound_Error tyrsound_shutdown()
{
    tyrsound::shutdownSounds();
    tyrsound::shutdownDevice();
    return TYRSOUND_ERR_OK;
}

tyrsound_Error tyrsound_setUpdateMutex(void *mutex, int (*lockFunc)(void*), void (*unlockFunc)(void*))
{
    tyrsound::s_updateMutex = mutex;
    tyrsound::s_lockMutexFunc = lockFunc;
    tyrsound::s_unlockMutexFunc = unlockFunc;

    return TYRSOUND_ERR_OK;
}

void tyrsound_setAlloc(tyrsound_Alloc allocFunc)
{
    tyrsound::g_alloc = allocFunc ? allocFunc : _defaultAlloc;
}



tyrsound_Error tyrsound_update(void)
{
    tyrsound::DeviceBase *device = tyrsound::getDevice();
    if(!device)
        return TYRSOUND_ERR_NO_DEVICE;

    device->update();
    tyrsound::updateSounds();
    return TYRSOUND_ERR_OK;
}

