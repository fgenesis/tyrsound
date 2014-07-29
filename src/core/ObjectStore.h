#ifndef TYRSOUND_OBJECTSTORE_H
#define TYRSOUND_OBJECTSTORE_H

#include "BasicClasses.h"

#include "tyrsound_begin.h"

// Handle bits:
// GGGGGGGG TTT00000 00000000 00000000
// 256 distinct generations (G)
// 8 possible types
// 2m possible objects

enum HandleBits
{
    // shift right by this many bits...
    OFFS_GENERATION = 24,
    OFFS_TYPE       = 21,
    OFFS_ID         = 0,

    // ... and mask with this to get the actual number
    MASK_GENERATION = 0x000000FF,
    MASK_TYPE       = 0x00000007,
    MASK_ID         = 0x001FFFFF

};

inline unsigned handle2generation(tyrsound_Handle h)
{
    return (h >> OFFS_GENERATION) & MASK_GENERATION;
}
inline Type handle2type(tyrsound_Handle h)
{
    return (Type)((h >> OFFS_TYPE) & MASK_TYPE);
}
inline unsigned handle2idx(tyrsound_Handle h)
{
    return ((h >> OFFS_ID) & MASK_ID) - 1; // For zero handles, this will underflow, resulting in an invalid index
}
inline tyrsound_Handle makehandle(unsigned gen, Type ty, unsigned id)
{
    return ((gen & MASK_GENERATION) << OFFS_GENERATION)
        | ((ty & MASK_TYPE) << OFFS_TYPE)
        | (id + 1);
}

class ObjectStore
{
    typedef void (*Destructor)(Referenced *ref);
public:
    ObjectStore(Type ty, Destructor f);
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

    Destructor dtor;

    bool getFreeIdx(unsigned *idxp) const;
};

#include "tyrsound_end.h"
#endif
