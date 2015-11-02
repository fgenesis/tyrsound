#include "tyrsound_internal.h"
#include "ObjectStore.h"
#include "ChannelGroup.h"

#include "tyrsound_begin.h"

static void destroyGroup(Referenced *ref)
{
    static_cast<ChannelGroup*>(ref)->destroy();
}

ObjectStore groupstore(TY_GROUP, destroyGroup);

tyrsound_Error initGroups()
{
    if(!groupstore.init())
        return TYRSOUND_ERR_OUT_OF_MEMORY;

    return TYRSOUND_ERR_OK;
}


static bool killGroup(ChannelGroup *group)
{
    bool ok = groupstore.remove(group);
    if(ok)
        groupstore.update();
    return ok;
}


void shutdownGroups()
{
    groupstore.update();
    
    for(unsigned i = 0; i < groupstore.size(); ++i)
        groupstore.remove(groupstore[i]);

    groupstore.update();
    
    if(groupstore.size())
        tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "shutdownGroups(): %u groups not deleted", groupstore.size());
    
    groupstore.clear();
}

static tyrsound_Error lookupGroup(tyrsound_Handle handle, ChannelGroup **pgroup)
{
    Referenced *ref = NULL;
    tyrsound_Error err = groupstore.lookupHandle(handle, &ref);
    *pgroup = static_cast<ChannelGroup*>(ref);
    return err;
}

#define LOOKUP_RET(var, h, ret)                    \
    tyrsound::ChannelGroup *var;                   \
    do { tyrsound_Error _err = tyrsound::lookupGroup(h, &var); \
    if(_err != TYRSOUND_ERR_OK || !var)            \
        return ret;                                \
    } while(0)

#define LOOKUP(var, h) LOOKUP_RET(var, h, _err)


#include "tyrsound_end.h"


TYRSOUND_DLL_EXPORT tyrsound_Group tyrsound_createGroup()
{
    tyrsound_Group handle = TYRSOUND_NULL_GROUP;

    tyrsound::ChannelGroup *g = tyrsound::ChannelGroup::create();
    if(g)
    {
        handle = (tyrsound_Group)tyrsound::groupstore.add(g);
        if(handle == TYRSOUND_NULL_GROUP)
            g->destroy();
        else
            tyrsound::groupstore.update();
    }
    return handle;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_deleteGroup(tyrsound_Group handle)
{
    LOOKUP(group, handle);
    if(!tyrsound::killGroup(group))
        return TYRSOUND_ERR_SHIT_HAPPENED;

    group->destroy();
    return TYRSOUND_ERR_OK;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_clearGroup(tyrsound_Group handle)
{
    LOOKUP(group, handle);
    group->clear();
    return TYRSOUND_ERR_OK;
}
