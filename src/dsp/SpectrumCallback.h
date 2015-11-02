#ifndef TYRSOUND_DSP_SPECTRUM_CALLBACK_H
#define TYRSOUND_DSP_SPECTRUM_CALLBACK_H

#include "DSPAPI.h"

struct kiss_fft_cpx;

#include "tyrsound_begin.h"

class SpectrumCallback : public DSPAPI
{
public:
    static SpectrumCallback *create();

    SpectrumCallback();
    ~SpectrumCallback();

    virtual void destroy();
    virtual void process(float samplerate, unsigned channels, void *state, float *v, size_t sz);
    virtual tyrsound_Error setParams(tyrsound_DSPParamPair *);
    virtual void *allocState(unsigned channels) const;

protected:
    tyrsound_Error _init(unsigned window);
    void _fftrun(unsigned seq, unsigned channels, const float *v, float *swizbuf, kiss_fft_cpx *cpx, float *freqbuf, float **ptrs, unsigned freqSize);

    void *fft;
    tyrsound_DSPCallback cb;
    unsigned windowSize;
    void *user;
};

#include "tyrsound_end.h"

#endif
