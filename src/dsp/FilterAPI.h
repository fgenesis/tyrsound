#ifndef TYRSOUND_FILTER_API_H
#define TYRSOUND_FILTER_API_H

#include "DSPAPI.h"

#include "tyrsound_begin.h"

template<typename Underlying>
class FilterAPI : public DSPAPI
{
public:
    /* override */ void process(float samplerate, unsigned channels, void *state, float *v, size_t sz);
    /* override */ tyrsound_Error setParams(tyrsound_DSPParamPair *);
    /* override */ void *allocState(unsigned channels) const;
    /* override */ void destroy();

    static FilterAPI<Underlying> *create();

protected:
    FilterAPI() {}

    Underlying filt;
};

template<typename Underlying>
FilterAPI<Underlying> *FilterAPI<Underlying>::create()
{
    void *p = Alloc(sizeof(FilterAPI<Underlying>));
    return p ? new(p) FilterAPI<Underlying>() : NULL;
}

template<typename Underlying>
void FilterAPI<Underlying>::destroy()
{
    this->~FilterAPI<Underlying>();
    Free(this);
}

template<typename Underlying>
void FilterAPI<Underlying>::process(float samplerate, unsigned channels, void *statep, float *v, size_t sz)
{
    filt.setSampleRate(samplerate);
    Underlying::State *state = (Underlying::State*)statep;
    for(unsigned i = 0; i < channels; ++i)
        filt.filter(state[i], v + i, sz, channels);
}

template<typename Underlying>
tyrsound_Error FilterAPI<Underlying>::setParams(tyrsound_DSPParamPair *pp)
{
    return filt.setParams(pp);
}

template<typename Underlying>
void *FilterAPI<Underlying>::allocState(unsigned channels) const
{
    const size_t sizeWithAlignment = ptrdiff_t((char*)(((Underlying::State*)NULL) + 1));
    void *mem = Alloc(sizeWithAlignment * channels);
    if(mem)
        new (mem) Underlying::State[channels];
    return mem;
}

#include "tyrsound_end.h"

#endif
