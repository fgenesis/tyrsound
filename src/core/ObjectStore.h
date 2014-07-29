#ifndef TYRSOUND_OBJECTSTORE_H
#define TYRSOUND_OBJECTSTORE_H

#include "BasicClasses.h"

#include "tyrsound_begin.h"

class ObjectStore
{
public:
    ObjectStore(Type ty);
    bool init();
    void update();
    tyrsound_Error lookupHandle(tyrsound_Handle handle, Referenced **pobj) const;
    tyrsound_Handle add(Referenced *obj);
    bool remove(Referenced *obj);
    void clear();

    inline size_t size() const { return objlist.size(); }
    inline Referenced* operator[](unsigned i) { return objlist[i]; }
    inline const Referenced* operator[](unsigned i) const { return objlist[i]; }

    PODArray<Referenced*> deadlist;

protected:

    const Type _type;

    // Members for plain storage
    PODArray<Referenced*> objlist;
    PODArray<Referenced*> addlist;
    PODArray<Referenced*> dellist;
    mutable Mutex *addmtx;
    mutable Mutex *delmtx;

    // Members for handle lookup
    struct Data
    {
        Referenced *obj;
        unsigned  generation;
    };
    PODArray<Data> _store;
    mutable Mutex *storemtx;

    bool getFreeIdx(unsigned *idxp) const;
};

#include "tyrsound_end.h"
#endif
