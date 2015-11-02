#ifndef TYRSOUND_IIRFILTER_H
#define TYRSOUND_IIRFILTER_H

#include <stddef.h>
#include <string.h>
#include "tyrsound_internal.h"
#include <stdio.h>

#include "tyrsound_begin.h"

template <unsigned N1, unsigned N2, typename Base>
class IIRFilter
{
public:
    enum { S1 = NextPowerOf2<N1>::value, MASK1 = S1 - 1 };
    enum { S2 = NextPowerOf2<N2>::value, MASK2 = S2 - 1 };
    struct State
    {
        State();
        // accessed often
        float in[S1]; // circular buffers, power-of-2 length so that index wrapping can be done cheaply
        float out[S2];
        // accessed rarely
        unsigned inPos;
        unsigned outPos;
        void reset();
    };

    void filter(State& state, float *v, size_t sz, unsigned spacing) const;

    inline void setSampleRate(float samplerate);

protected:
    IIRFilter();
    float a[N1]; // filter coefficients
    float b[N2];
    float freq;

    FORCE_INLINE float getSampleRate() const { return sr; }

private:
    float sr;
};

template <unsigned N1, unsigned N2, typename Base>
IIRFilter<N1, N2, Base>::IIRFilter()
: sr(0.0f)
, freq(0.0f)
{
}

template <unsigned N1, unsigned N2, typename Base>
IIRFilter<N1, N2, Base>::State::State()
{
    reset();
}

template <unsigned N1, unsigned N2, typename Base>
void IIRFilter<N1, N2, Base>::State::reset()
{
    memset(this, 0, sizeof(*this));
}


template <unsigned N1, unsigned N2, typename Base>
void IIRFilter<N1, N2, Base>::setSampleRate(float samplerate)
{
    if(sr != samplerate)
    {
        sr = samplerate;
        ((Base*)this)->setupCoeffs();
    }
}

template <unsigned N1, unsigned N2, typename Base>
void IIRFilter<N1, N2, Base>::filter(State& state, float *v, size_t sz, unsigned spacing) const
{
    // make sure these are in registers
    unsigned inPos = state.inPos;
    unsigned outPos = state.outPos;

    for(size_t i = 0; i < sz; i += spacing)
    {
        state.in[inPos] = v[i];
        float sample = 0;
        unsigned x = inPos;
        #pragma unroll
        for(unsigned j = 0; j < N1; ++j, --x)
            sample += a[j] * state.in[x & MASK1];
        x = outPos;
        #pragma unroll
        for(unsigned j = 0; j < N2; ++j, --x)
            sample += b[j] * state.out[x & MASK2];

        outPos = (outPos + 1) & MASK2;
        inPos = (inPos + 1) & MASK1;
        state.out[outPos] = sample;
        v[i] = sample;
    }

    state.inPos = inPos;
    state.outPos = outPos;
}


// ----------------------------------

template<typename Base> class ResonantLowpassFilterT
{
public:
    struct State
    {
        State();
        float speed;
        float pos;
        void reset();
    };
    void filter(State& state, float *v, size_t sz, unsigned spacing) const;
    tyrsound_Error setParams(tyrsound_DSPParamPair *pp);
    inline void setSampleRate(float samplerate);

protected:
    ResonantLowpassFilterT();
    float C;
    float R;
    float sr;
    float freq;
    float amp;
};

template<typename Base>
ResonantLowpassFilterT<Base>::ResonantLowpassFilterT()
: C(0.0f)
, R(0.0f)
, sr(0.0f)
, freq(0.0f)
, amp(0.0f)
{
}

template<typename Base>
void ResonantLowpassFilterT<Base>::State::reset()
{
    speed = 0.0f;
    pos = 0.0f;
}

template<typename Base>
ResonantLowpassFilterT<Base>::State::State()
{
    reset();
}

template <typename Base>
void ResonantLowpassFilterT<Base>::setSampleRate(float samplerate)
{
    if(sr != samplerate)
    {
        sr = samplerate;
        ((Base*)this)->setupCoeffs();
    }
}

template<typename Base>
void ResonantLowpassFilterT<Base>::filter(State &state, float *v, size_t sz, unsigned spacing) const
{
    float speed = state.speed;
    float pos = state.pos;
    const float R = this->R;
    const float C = this->C;

    for(size_t i = 0; i < sz; i += spacing)
    {
        speed += (v[i] - pos) * C;
        pos += speed;
        speed *= R;
        v[i] = pos;
    }

    state.speed = speed;
    state.pos = pos;
}

template<typename Base>
tyrsound_Error ResonantLowpassFilterT<Base>::setParams(tyrsound_DSPParamPair *pp)
{
    tyrsound_Error err = TYRSOUND_ERR_OK;
    for( ; pp->id; ++pp)
    {
        switch(pp->id)
        {
        case TYRSOUND_DSPPARAM_CUTOFF:
            freq = pp->val.f;
            break;

        case TYRSOUND_DSPPARAM_RESONANCE:
            amp = vmax(0.2f, pp->val.f); // FIXME: HACK: values any smaller than this cause filter instability
            break;

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "ResonantLowpassFilterBase: Unsupported param: [%u]", pp->id);
            err = TYRSOUND_ERR_UNSUPPORTED;
        }
    }
    ((Base*)this)->setupCoeffs();
    return err;
}



#include "tyrsound_end.h"

#endif
