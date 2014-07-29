#include "tyrsound_internal.h"
#include "ObjectStore.h"
#include "BasicClasses.h"

#include "tyrsound_begin.h"


ObjectStore::ObjectStore(Type ty, Destructor f)
: _type(ty)
, addmtx(NULL)
, delmtx(NULL)
, storemtx(NULL)
, dtor(f)
{
    // Don't to anything else in here, as we're probably in the static init phase
}

bool ObjectStore::init()
{
    return objlist.reserve(64)
        && _store.reserve(64)
        && addlist.reserve(16)
        && dellist.reserve(16)
        && (dtor || deadlist.reserve(16))
        && (!tyrsound_ex_hasMT() || (
               (storemtx = Mutex::create())
            && (addmtx  = Mutex::create())
            && (delmtx  = Mutex::create())
           )
        );
}

void ObjectStore::clear()
{
    objlist.clear();
    _store.clear();
    addlist.clear();
    dellist.clear();
    deadlist.clear();
    if(storemtx)
    {
        storemtx->destroy();
        storemtx = NULL;
    }
    if(addmtx)
    {
        addmtx->destroy();
        addmtx = NULL;
    }
    if(delmtx)
    {
        delmtx->destroy();
        delmtx = NULL;
    }
}

bool ObjectStore::getFreeIdx(unsigned *idxp) const
{
    for(unsigned  i = 0; i < _store.size(); ++i)
        if(!_store[i].obj)
        {
            *idxp = i;
            return true;
        }
    return false;
}

tyrsound_Error ObjectStore::lookupHandle(tyrsound_Handle handle, Referenced **pobj) const
{
    unsigned idx = handle2idx(handle);
    if(idx >= MASK_ID)
    {
        *pobj = NULL;
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Invalid/Malformed handle");
        return TYRSOUND_ERR_INVALID_HANDLE;
    }

    const Type ty = handle2type(handle);
    if(ty != _type)
    {
        *pobj = NULL;
        tyrsound_ex_messagef(TYRSOUND_MSG_ERROR, "Handle is of the wrong type (is: %u, should be: %u)", ty, _type);
        return TYRSOUND_ERR_INVALID_HANDLE;
    }

    const unsigned generation = handle2generation(handle);
    Data data;

    // -- accessing member variables in this block --
    {
        MutexGuard guard(storemtx);
        if(!guard)
            return TYRSOUND_ERR_UNSPECIFIED;

        if(idx >= _store.size())
        {
            guard.unlock(); // avoid calling the message callback with a lock held
            *pobj = NULL;
            tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Invalid handle (out of bounds index)");
            return TYRSOUND_ERR_INVALID_HANDLE;
        }
        data = _store[idx];
    }

    if(data.generation != generation || data.obj->_idxInStore != idx || data.obj->_dead)
    {
        *pobj = NULL;
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Invalid handle (already deleted?)");
        return TYRSOUND_ERR_INVALID_HANDLE;
    }

    *pobj = data.obj;
    return TYRSOUND_ERR_OK;
}

tyrsound_Handle ObjectStore::add(Referenced *obj)
{
    unsigned generation, idx;
    {
        MutexGuard guard(storemtx);
        if(!guard)
            return TYRSOUND_NULL_HANDLE;

        if(getFreeIdx(&idx))
        {
            generation = _store[idx].generation;
            _store[idx].obj = obj;
        }
        else
        {
            Data d;
            d.generation = generation = 0;
            d.obj = obj;
            if(!_store.push_back(d))
                return TYRSOUND_NULL_HANDLE;
            idx = _store.size() - 1;
        }

        obj->_idxInStore = idx;
    }

    {
        MutexGuard guard(addmtx);
        if(!guard)
            return TYRSOUND_NULL_HANDLE;

        if(!addlist.push_back(obj))
            return TYRSOUND_NULL_HANDLE;
    }

    return makehandle(generation, _type, idx);
}

bool ObjectStore::remove(Referenced *obj)
{
    if(obj->_type != _type)
    {
        tyrsound_ex_messagef(TYRSOUND_MSG_INTERNAL_ERROR, "ObjectStore::remove(): Handle is of the wrong type (is: %u, should be: %u)", obj->_type, _type);
        return false;
    }
    const unsigned idxInStore = obj->_idxInStore;
    obj->_idxInStore = unsigned(-1);
    obj->_dead = true;

    {
        MutexGuard guard(storemtx);
        if(!guard)
            return false;

        if(_store[idxInStore].obj != obj)
        {
            guard.unlock(); // avoid calling the message callback with a lock held
            tyrsound_ex_messagef(TYRSOUND_MSG_INTERNAL_ERROR, "ObjectStore::remove(): _store[idxInStore].obj != obj. idxInStore = %u", idxInStore);
            return false;
        }

        _store[idxInStore].obj = NULL;
        _store[idxInStore].generation++;
    }

    {
        MutexGuard guard(delmtx);
        if(!guard)
            return false;

        if(!dellist.push_back(obj))
            return false;
    }

    return true;
}

void ObjectStore::update()
{
    Referenced *obj;

    {
        MutexGuard guard(delmtx);

        while(dellist.pop_back(obj))
        {
            const unsigned idxInList = obj->_idxInList;
            obj->_idxInList = unsigned(-1);
            if(objlist[idxInList] != obj)
            {
                tyrsound_ex_messagef(TYRSOUND_MSG_INTERNAL_ERROR, "ObjectStore::update(): objlist[idxInList] != obj. idxInList = %u", idxInList);
            }

            Referenced *e = objlist.pop_back();
            if(e != obj)
            {
                objlist[idxInList] = e;
                e->_idxInList = idxInList;
            }

            obj->_dead = true;
            if(dtor)
                dtor(obj);
            else
                deadlist.push_back(obj);
        }
    }

    {
        MutexGuard guard(addmtx);

        while(addlist.pop_back(obj))
        {
            objlist.push_back(obj);
            obj->_idxInList = objlist.size() - 1;
        }
    }
}



#include "tyrsound_end.h"
