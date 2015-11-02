#include <math.h>
#include "SpectrumCallback.h"
#include "tyrsound_internal.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"
#include <assert.h>

#include "tyrsound_begin.h"

SpectrumCallback *SpectrumCallback::create()
{
    void *mem = Alloc(sizeof(SpectrumCallback));
    if(!mem)
        return NULL;
    return new (mem) SpectrumCallback();
}

SpectrumCallback::SpectrumCallback()
: fft(NULL)
, cb(NULL)
, windowSize(0)
, user(NULL)
{
    _init(1024);
}

SpectrumCallback::~SpectrumCallback()
{
    Free(fft);
}

tyrsound_Error SpectrumCallback::_init(unsigned window)
{
    if(!window)
        return TYRSOUND_ERR_INVALID_VALUE;
    if(window == windowSize)
        return TYRSOUND_ERR_OK;
    window = nextPowerOf2(window);
    windowSize = window;
    size_t req = 0;
    kiss_fftr_alloc(window, 0, NULL, &req);
    void *mem = Alloc(req);
    fft = kiss_fftr_alloc(window, 0, mem, &req);
    if(!fft)
        return TYRSOUND_ERR_OUT_OF_MEMORY;

    return TYRSOUND_ERR_OK;
}

void *SpectrumCallback::allocState(unsigned channels) const
{
    return NULL;
}

void SpectrumCallback::destroy()
{
    this->~SpectrumCallback();
    Free(this);
}

static void mag(float *dst, kiss_fft_cpx *src, size_t sz)
{
    for(size_t i = 0; i < sz; ++i)
    {
        const float re = src[i].r;
        const float im = src[i].i;
        dst[i] = sqrtf(re*re + im*im);
    }
}

void SpectrumCallback::_fftrun(unsigned seq, unsigned channels, const float *v, float *swizbuf, kiss_fft_cpx *cpx, float *freqbuf, float **ptrs, unsigned freqSize)
{
    for(unsigned i = 0; i < channels; ++i)
    {
        unsigned r = i;
        for(unsigned w = 0; w < windowSize; ++w)
        {
            swizbuf[w] = v[r];
            r += channels;
        }
        kiss_fftr((kiss_fftr_cfg)fft, swizbuf, cpx);

        float * const wp = &freqbuf[i * freqSize];
        mag(wp, cpx, freqSize);
        ptrs[i] = wp;
    }

    cb(ptrs, channels, freqSize, seq, user);
}

void SpectrumCallback::process(float samplerate, unsigned channels, void *state, float *v, size_t sz)
{
    (void)samplerate;
    (void)state;

    if(!cb)
        return;

    const unsigned windowSize = this->windowSize;
    const unsigned blocksize = windowSize * channels;
    const unsigned freqSize = windowSize / 2 + 1;

    const unsigned memSizePtrs = sizeof(float*) * channels;
    const unsigned memSizeFreqBuf = sizeof(float) * channels * freqSize;
    const unsigned memSizeSwizbuf = sizeof(float) * windowSize;
    const unsigned memSizeCpx = sizeof(kiss_fft_cpx) * freqSize;

    const unsigned memSize = memSizePtrs + memSizeSwizbuf + memSizeCpx + memSizeFreqBuf;
    char *mem = (char*)TYRSOUND_STACK_ALLOC(memSize);

    float **ptrs = (float**)mem;
    float *freqbuf = (float*)((char*)ptrs + memSizePtrs);
    float *swizbuf = (float*)((char*)freqbuf + memSizeFreqBuf);
    kiss_fft_cpx *cpx = (kiss_fft_cpx*)((char*)swizbuf + memSizeSwizbuf);

    const unsigned blocks = sz / blocksize;
    for(unsigned seq = 0; seq < blocks; ++seq)
    {
        const float *start = v + seq * blocksize;
        _fftrun(seq, channels, start, swizbuf, cpx, freqbuf, ptrs, freqSize);
    }

    const unsigned remain = sz - blocks * blocksize;
    if(remain)
    {

    }

    TYRSOUND_STACK_FREE(mem);
}

tyrsound_Error SpectrumCallback::setParams(tyrsound_DSPParamPair *pp)
{
    tyrsound_Error err = TYRSOUND_ERR_OK;
    for( ; pp->id; ++pp)
    {
        switch(pp->id)
        {
        case TYRSOUND_DSPPARAM_WINDOW_SIZE:
            return _init(pp->val.i);

        case TYRSOUND_DSPPARAM_USERDATA:
            user = pp->val.p;
            break;

        case TYRSOUND_DSPPARAM_CALLBACK:
            cb = (tyrsound_DSPCallback)pp->val.p;
            break;

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "SpectrumCallback: Unsupported param: [%u] = %f", pp->id, pp->val);
            err = TYRSOUND_ERR_UNSUPPORTED;
        }
    }
    return err;
}

#include "tyrsound_end.h"
