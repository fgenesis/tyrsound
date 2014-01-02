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

    return tyrsound::initSounds();
}

tyrsound_Error tyrsound_shutdown()
{
    tyrsound::shutdownSounds();
    tyrsound::shutdownDevice();
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

