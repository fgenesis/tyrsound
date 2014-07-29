#include "tyrsound_internal.h"
#include "BasicClasses.h"
#include "ObjectStore.h"


#include "tyrsound_begin.h"

static tyrsound_Error lookupPlayable(tyrsound_Handle h, Playable **obj)
{
    tyrsound_Error err = TYRSOUND_ERR_INVALID_HANDLE;
    Referenced *p = NULL;
    switch(handle2type(h))
    {
        case TY_SOUND: err = soundstore.lookupHandle(h, &p); break;
        case TY_GROUP: err = groupstore.lookupHandle(h, &p); break;
        default:
            tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Handle is not of a playable type");
    }
    *obj = static_cast<ReferencedPlayable*>(p);
    return err;
}


#include "tyrsound_end.h"

#define LOOKUP_RET(var, h, ret)                    \
    tyrsound::Playable *var;                       \
    do { tyrsound_Error _err = tyrsound::lookupPlayable(h, &var); \
    if(_err != TYRSOUND_ERR_OK || !var)            \
        return ret;                                \
    } while(0)

#define LOOKUP(var, h) LOOKUP_RET(var, h, _err)


TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setVolume(tyrsound_Handle handle, float vol)
{
    LOOKUP(obj, handle);
    return obj->setVolume(vol);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setSpeed(tyrsound_Handle handle, float speed)
{
    LOOKUP(obj, handle);
    return obj->setSpeed(speed);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setPosition(tyrsound_Handle handle, float x, float y, float z)
{
    LOOKUP(obj, handle);
    return obj->setPosition(x, y, z);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_play(tyrsound_Handle handle)
{
    LOOKUP(obj, handle);
    return obj->play();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_pause(tyrsound_Handle handle)
{
    LOOKUP(obj, handle);
    return obj->pause();
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_stop(tyrsound_Handle handle)
{
    LOOKUP(obj, handle);
    return obj->stop();
}

TYRSOUND_DLL_EXPORT int tyrsound_isPlaying(tyrsound_Handle handle)
{
    LOOKUP_RET(obj, handle, 0);
    return obj->isPlaying();
}

