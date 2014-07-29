#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "ObjectStore.h"

#include "tyrsound_begin.h"


static ObjectStore soundstore(TY_SOUND);

tyrsound_Sound registerSoundObject(SoundObject *sound)
{
    return static_cast<tyrsound_Sound>(soundstore.add(sound));
}

static tyrsound_Error unregisterSoundObject(SoundObject *sound)
{
    if(!soundstore.remove(sound))
        return TYRSOUND_ERR_SHIT_HAPPENED;

    return TYRSOUND_ERR_OK;
}

tyrsound_Error initSounds()
{
    if(!soundstore.init())
        return TYRSOUND_ERR_OUT_OF_MEMORY;

    return TYRSOUND_ERR_OK;
}

static void deleteDead()
{
    if(const unsigned sz = soundstore.deadlist.size())
    {
        //tyrsound_ex_messagef(TYRSOUND_MSG_DEBUG, "Deleting %u dead sounds", sz);
        for(unsigned i = 0; i < sz; ++i)
            ((SoundObject*)soundstore.deadlist[i])->destroy();
        soundstore.deadlist.resize(0); // not clear() to avoid actually freeing the memory
    }
}

void shutdownSounds()
{
    // clear add list
    soundstore.update();

    const unsigned alive = soundstore.size();
    for(unsigned i = 0; i < soundstore.size(); ++i)
    {
        SoundObject *sound = (SoundObject*)soundstore[i];
        sound->stop();
        tyrsound::unregisterSoundObject(sound);
    }

    // clear delete list
    soundstore.update();

    // this should delete all of them
    deleteDead();

    const unsigned stillAlive = soundstore.size();
    if(alive || stillAlive)
        tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "Shutting down while %u sounds were still active. %u sounds not deleted.", alive, stillAlive);
    soundstore.clear();
}

tyrsound_Error updateSounds()
{
    soundstore.update();

    for(unsigned i = 0; i < soundstore.size(); ++i)
    {
        SoundObject *sound = (SoundObject*)soundstore[i];
        sound->update();
        if(sound->isAutoFree() && sound->isStopped())
            unregisterSoundObject(sound);
    }

    deleteDead();

    return TYRSOUND_ERR_OK;
}


#include "tyrsound_end.h"

#define LOOKUP_RET(var, h, ret)                    \
    tyrsound::SoundObject *var = NULL;             \
    do { tyrsound::Referenced *ref;                \
        tyrsound_Error _err = tyrsound::soundstore.lookupHandle(h, &ref); \
        if(!ref || _err != TYRSOUND_ERR_OK)        \
            return ret;                            \
        var = (tyrsound::SoundObject*)ref;         \
    } while(0)

#define LOOKUP(var, h) LOOKUP_RET(var, h, _err)

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_unload(tyrsound_Sound handle)
{
    LOOKUP(sound, handle);
    sound->stop();
    return tyrsound::unregisterSoundObject(sound);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setVolume(tyrsound_Handle handle, float vol)
{
    LOOKUP(sound, handle);
    return sound->setVolume(vol);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setSpeed(tyrsound_Sound handle, float speed)
{
    LOOKUP(sound, handle);
    return sound->setSpeed(speed);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_seek(tyrsound_Sound handle, float seconds)
{
    LOOKUP(sound, handle);
    return sound->seek(seconds);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setPosition(tyrsound_Handle handle, float x, float y, float z)
{
    LOOKUP(sound, handle);
    return sound->setPosition(x, y, z);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setLoop(tyrsound_Sound handle, float seconds, int loops)
{
    LOOKUP(sound, handle);
    return sound->setLoop(seconds, loops);
}

TYRSOUND_DLL_EXPORT float tyrsound_getLength(tyrsound_Sound handle)
{
    LOOKUP_RET(sound, handle, -1.0f);
    return sound->getLength();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_play(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    return sound->play();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_pause(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    return sound->pause();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_stop(tyrsound_Handle handle)
{
    LOOKUP(sound, handle);
    return sound->stop();
}

TYRSOUND_DLL_EXPORT int tyrsound_isPlaying(tyrsound_Handle handle)
{
    LOOKUP_RET(sound, handle, 0);
    return sound->isPlaying();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_autoFree(tyrsound_Sound handle)
{
    LOOKUP_RET(sound, handle, _err);
    sound->setAutoFree(true);
    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT float tyrsound_getPlayPosition(tyrsound_Sound handle)
{
    LOOKUP_RET(sound, handle, 0);
    return sound->getPlayPosition();
}

