#include "tyrsound_internal.h"
#include "SoundObject.h"

#include "tyrsound_begin.h"


struct ObjectData
{
    SoundObject *obj;
    unsigned int generation;
};

static ObjectData *objectStore = NULL;
static unsigned int objectStoreCapacity = 0;
static Mutex *updateMutex = NULL;
static SoundObject *updateObjRoot = NULL;

static bool getFreeIdx(unsigned int *idxp)
{
    for(unsigned int i = 0; i < objectStoreCapacity; ++i)
        if(!objectStore[i].obj)
        {
            *idxp = i;
            return true;
        }
    return false;
}

static bool enlargeStore(unsigned int *idxp)
{
    unsigned int newCap = objectStoreCapacity + (objectStoreCapacity >> 1) + 8;
    ObjectData *newStore = (ObjectData*)Realloc(objectStore, newCap * sizeof(ObjectData));
    if(!newStore)
        return false;

    *idxp = objectStoreCapacity; // first free idx
    memset(newStore + objectStoreCapacity, 0, (newCap - objectStoreCapacity) * sizeof(ObjectData));
    objectStore = newStore;
    objectStoreCapacity = newCap;
    return true;
}

static tyrsound_Error lookupHandle(tyrsound_Handle handle, SoundObject **soundp, bool allowNull = false)
{
    unsigned int idx = (handle & 0xFFFFFF); // mask out generation
    if(!idx)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Invalid/Malformed handle");
        return TYRSOUND_ERR_INVALID_HANDLE;
    }

    --idx;
    unsigned int generation = handle >> 24;

    const ObjectData data = objectStore[idx];
    if(data.generation != generation || data.obj->_idxInStore != idx || data.obj->_dead)
    {
        *soundp = NULL;
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Invalid handle (already deleted?)");
        return TYRSOUND_ERR_INVALID_HANDLE;
    }

    *soundp = data.obj;
    return TYRSOUND_ERR_OK;
}

tyrsound_Handle registerSoundObject(SoundObject *sound)
{
    MutexGuard guard(updateMutex);
    if(!guard)
        return 0;

    unsigned int idx;
    if(!getFreeIdx(&idx))
    {
        if(!enlargeStore(&idx))
            return TYRSOUND_ERR_OUT_OF_MEMORY;
    }

    const unsigned int generation = objectStore[idx].generation;
    objectStore[idx].obj = sound;
    sound->_idxInStore = idx;
    sound->_dead = false;
    
    return (idx + 1) | (generation << 24); // 8 bits for generation should be enough
}

static tyrsound_Error unregisterSoundObject(SoundObject *sound)
{
    MutexGuard guard(updateMutex);
    if(!guard)
        return TYRSOUND_ERR_UNSPECIFIED;

    const unsigned int idx = sound->_idxInStore;
    sound->_idxInStore = unsigned(-1);

    if(objectStore[idx].obj != sound)
        return TYRSOUND_ERR_SHIT_HAPPENED;

    objectStore[idx].obj = NULL;
    objectStore[idx].generation++;

    return TYRSOUND_ERR_OK;
}

void registerUpdate(SoundObject *sound)
{
    MutexGuard guard(updateMutex);
    sound->_update = true;
}

void unregisterUpdate(SoundObject *sound)
{
    MutexGuard guard(updateMutex);
    sound->_update = false;
}

static tyrsound_Error destroySoundObject(SoundObject *sound)
{
    tyrsound_Error err = unregisterSoundObject(sound);
    sound->destroy();
    return err;
}

tyrsound_Error initSounds()
{
    updateMutex = Mutex::create();
    if(!updateMutex && tyrsound_ex_hasMT())
        return TYRSOUND_ERR_OUT_OF_MEMORY;


    return TYRSOUND_ERR_OK;
}

void shutdownSounds()
{
    for(unsigned int i = 0; i < objectStoreCapacity; ++i)
        if(SoundObject *sound = objectStore[i].obj)
            sound->destroy();
    Free(objectStore);
    objectStore = NULL;
    objectStoreCapacity = 0;
    if(updateMutex)
    {
        updateMutex->destroy();
        updateMutex = NULL;
    }
}

tyrsound_Error updateSounds()
{
    for(unsigned int i = 0; i < objectStoreCapacity; ++i)
        if(SoundObject *sound = objectStore[i].obj)
        {
            if(sound->_dead)
                destroySoundObject(sound);
            else
            {
                MutexGuard guard(updateMutex);
                sound->update();
            }
        }

    return TYRSOUND_ERR_OK;
}

static tyrsound_Error enqueueDeletion(SoundObject *sound)
{
    sound->_dead = true;
    return TYRSOUND_ERR_OK;
}


#include "tyrsound_end.h"

#define LOOKUP_RET(var, h, ret)                    \
    tyrsound::SoundObject *var;                    \
    do {                                           \
        tyrsound_Error _err = tyrsound::lookupHandle(h, &var); \
        if(!var || _err != TYRSOUND_ERR_OK)        \
            return ret;                            \
    } while(0)

#define LOOKUP(var, h) LOOKUP_RET(var, h, _err)

tyrsound_Error tyrsound_unload(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    sound->stop();
    return tyrsound::enqueueDeletion(sound);
}

tyrsound_Error tyrsound_setVolume(tyrsound_Handle handle, float vol)
{
    LOOKUP(sound, handle);
    return sound->setVolume(vol);
}

tyrsound_Error tyrsound_setSpeed(tyrsound_Handle handle, float speed)
{
    LOOKUP(sound, handle);
    return sound->setSpeed(speed);
}

tyrsound_Error tyrsound_seek(tyrsound_Handle handle, float seconds)
{
    LOOKUP(sound, handle);
    return sound->seek(seconds);
}

tyrsound_Error tyrsound_setPosition(tyrsound_Handle handle, float x, float y, float z)
{
    LOOKUP(sound, handle);
    return sound->setPosition(x, y, z);
}

tyrsound_Error tyrsound_setLoop(tyrsound_Handle handle, float seconds, int loops)
{
    LOOKUP(sound, handle);
    return sound->setLoop(seconds, loops);
}

float tyrsound_getLength(tyrsound_Handle handle)
{
    LOOKUP_RET(sound, handle, -1.0f);
    return sound->getLength();
}

tyrsound_Error tyrsound_play(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    return sound->play();
}

tyrsound_Error tyrsound_pause(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    return sound->pause();
}

tyrsound_Error tyrsound_stop(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    return sound->stop();
}

int tyrsound_isPlaying(tyrsound_Handle handle)
{
    LOOKUP_RET(sound, handle, 0);
    return sound->isPlaying();
}

float tyrsound_getPlayPosition(tyrsound_Handle handle)
{
    LOOKUP_RET(sound, handle, 0);
    return sound->getPlayPosition();
}

