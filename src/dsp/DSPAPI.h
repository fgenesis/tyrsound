#ifndef TYRSOUND_DSP_API_H
#define TYRSOUND_DSP_API_H

#include "BasicClasses.h"

#include "tyrsound_begin.h"

class DSPAPI : public Referenced
{
public:
    virtual void destroy() = 0;
    virtual void process(float samplerate, unsigned channels, void *state, float *v, size_t sz) = 0;
    virtual tyrsound_Error setParams(tyrsound_DSPParamPair *) = 0;
    virtual void *allocState(unsigned channels) const = 0;

protected:
    FORCE_INLINE DSPAPI() : Referenced(TY_DSP) {}
};


#include "tyrsound_end.h"

#endif

