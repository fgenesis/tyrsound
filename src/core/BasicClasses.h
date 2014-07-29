#ifndef TYRSOUND_BASIC_CLASSES_H
#define TYRSOUND_BASIC_CLASSES_H
#include "tyrsound_internal.h"
#include "tyrsound_begin.h"

class ObjectStore;

class Referenced
{
    friend class ObjectStore;
protected:
    inline Referenced(Type ty) : _idxInStore(unsigned(-1)), _idxInList(unsigned(-1)), _type(ty), _dead(false) {}
private:
    unsigned _idxInStore;
    unsigned _idxInList;
    Type _type;
    bool _dead;
};

class Playable
{
protected:
    inline Playable() {}
    virtual ~Playable() {}

public:
    virtual tyrsound_Error setVolume(float vol) = 0;
    virtual tyrsound_Error setSpeed(float speed) = 0;
    virtual tyrsound_Error stop() = 0;
    virtual tyrsound_Error play() = 0;
    virtual tyrsound_Error pause() = 0;
    virtual tyrsound_Error setPosition(float x, float y, float z) = 0;
    virtual bool isPlaying() = 0;
    virtual bool isStopped() = 0;
};

// very simple std::vector replacement for POD types
// some changed return values to signal out-of-memory
template<typename T> class PODArray
{
public:
    PODArray() : _arr(NULL), _sz(0), _capacity(0) {}
    ~PODArray() { clear(); }
    void clear() { Free(_arr); _arr = NULL; _sz = 0; _capacity = 0; }
    inline unsigned size() const { return _sz; }
    inline bool resize(unsigned n)
    {
        if(!reserve(n))
            return false;
        _sz = n;
        return true;
    }
    inline bool reserve(unsigned n)
    {
        return _capacity >= n || _alloc(n);
    }
    inline unsigned remain() const { return _capacity - _sz; }
    inline T pop_back() { return _arr[--_sz]; }
    inline bool pop_back(T& e)
    {
        if(!_sz)
            return false;
        e = _arr[--_sz];
        return true;
    }
    inline bool push_back(const T& e)
    {
        if(_sz >= _capacity)
            if(!reserve(_capacity + (_capacity >> 1) + 4))
                return false;
        _arr[_sz++] = e;
        return true;
    }
    inline T& operator[](unsigned i) { return _arr[i]; }
    inline const T& operator[](unsigned i) const { return _arr[i]; }
private:
    bool _alloc(unsigned n)
    {
        T* p = (T*)Realloc(_arr, n * sizeof(T));
        if(!p)
            return false;
        _arr = p;
        _capacity = n;
        return true;
    }
    T *_arr;
    unsigned _sz;
    unsigned _capacity;
};


#include "tyrsound_end.h"
#endif
