#include "tyrsound.h"
#include "tyrsound_internal.h"
#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

// Global state
static tyrsound_Alloc s_alloc = NULL;
static void *s_alloc_user = NULL;

static void *s_updateMutex = NULL;
static int (*s_lockMutexFunc)(void*) = NULL;
static void (*s_unlockMutexFunc)(void*) = NULL;


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


void *tyrsound_ex_alloc(void *ptr, size_t size)
{
    if(s_alloc)
        return s_alloc(ptr, size, s_alloc_user);

    return realloc(ptr, size);
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

    return haveDevice ? TYRSOUND_ERR_OK : TYRSOUND_ERR_NO_DEVICE;
}

tyrsound_Error tyrsound_shutdown()
{
    tyrsound::shutdownSounds();
    tyrsound::shutdownDevice();
    tyrsound_setAlloc(NULL, NULL);
    return TYRSOUND_ERR_OK;
}

tyrsound_Error tyrsound_setUpdateMutex(void *mutex, int (*lockFunc)(void*), void (*unlockFunc)(void*))
{
    tyrsound::s_updateMutex = mutex;
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
    tyrsound::updateSounds();
    return TYRSOUND_ERR_OK;
}

