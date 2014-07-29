#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "ObjectStore.h"
#include "ChannelGroup.h"

#include "tyrsound_begin.h"


ObjectStore soundstore(TY_SOUND, NULL);

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



static tyrsound_Error lookupSound(tyrsound_Handle handle, SoundObject **psound)
{
    Referenced *ref = NULL;
    tyrsound_Error err = soundstore.lookupHandle(handle, &ref);
    *psound = static_cast<SoundObject*>(ref);
    return err;
}


#include "tyrsound_end.h"

#define LOOKUP_RET(var, h, ret)                    \
    tyrsound::SoundObject *var;                    \
    do { tyrsound_Error _err = tyrsound::lookupSound(h, &var); \
    if(_err != TYRSOUND_ERR_OK || !var)            \
        return ret;                                \
    } while(0)

#define LOOKUP(var, h) LOOKUP_RET(var, h, _err)

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_unload(tyrsound_Sound handle)
{
    LOOKUP(sound, handle);
    sound->stop();
    return tyrsound::unregisterSoundObject(sound);
}


TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_seek(tyrsound_Sound handle, float seconds)
{
    LOOKUP(sound, handle);
    return sound->seek(seconds);
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

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_fireAndForget(tyrsound_Sound handle)
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

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setGroup(tyrsound_Sound handle, tyrsound_Group grouphandle)
{
    LOOKUP(sound, handle);
    tyrsound::Referenced *gref = NULL;
    if(grouphandle != TYRSOUND_NULL_GROUP)
    {
        tyrsound_Error err = tyrsound::groupstore.lookupHandle(grouphandle, &gref);
        if(err != TYRSOUND_ERR_OK || !gref)
            return err;
        tyrsound::ChannelGroup *group = static_cast<tyrsound::ChannelGroup*>(gref);
        return group->attachSound(sound);
    }

    sound->detachFromGroup();
    return TYRSOUND_ERR_OK;
}
