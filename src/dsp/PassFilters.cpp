#include "PassFilters.h"
#include "tyrsound_internal.h"
#include <math.h>

#include "tyrsound_begin.h"

void LowpassFilter::setupCoeffs()
{
    const float f = freq / getSampleRate();
    float x = exp(-14.445f * f);
    const float xsq = x * x;
    a[0] = pow(1.0f - x, 4);
    b[0] = 4.0f * x;
    b[1] = -6.0f * xsq;
    b[2] = b[0] * xsq;
    b[3] = -(xsq * xsq);
}

tyrsound_Error LowpassFilter::setParams(tyrsound_DSPParamPair *pp)
{
    tyrsound_Error err = TYRSOUND_ERR_OK;
    for( ; pp->id; ++pp)
    {
        switch(pp->id)
        {
        case TYRSOUND_DSPPARAM_CUTOFF:
            freq = pp->val.f;
            break;

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "LowpassFilter: Unsupported param: [%u]", pp->id);
            err = TYRSOUND_ERR_UNSUPPORTED;
        }
    }
    setupCoeffs();
    return err;
}


void CheapLowpassFilter::setupCoeffs()
{
    b[0] = expf((freq / getSampleRate()) * -6.283185307179586f);
    a[0] = 1.0f - b[0];
}

tyrsound_Error CheapLowpassFilter::setParams(tyrsound_DSPParamPair *pp)
{
    tyrsound_Error err = TYRSOUND_ERR_OK;
    for( ; pp->id; ++pp)
    {
        switch(pp->id)
        {
        case TYRSOUND_DSPPARAM_CUTOFF:
            freq = pp->val.f;
            break;

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "CheapLowpassFilter: Unsupported param: [%u]", pp->id);
            err = TYRSOUND_ERR_UNSUPPORTED;
        }
    }
    setupCoeffs();
    return err;
}


void HighpassFilter::setupCoeffs()
{
    b[0] = expf((freq / getSampleRate()) * -6.283185307179586f);
    a[0] = 0.5f * (1.0f + b[0]);
    a[1] = -a[0];
}

tyrsound_Error HighpassFilter::setParams(tyrsound_DSPParamPair *pp)
{
    tyrsound_Error err = TYRSOUND_ERR_OK;
    for( ; pp->id; ++pp)
    {
        switch(pp->id)
        {
        case TYRSOUND_DSPPARAM_CUTOFF:
        case TYRSOUND_DSPPARAM_CUTOFF_HIGH: // for compat
            freq = pp->val.f;
            break;

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "HighpassFilter: Unsupported param: [%u]", pp->id);
            err = TYRSOUND_ERR_UNSUPPORTED;
        }
    }
    setupCoeffs();
    return err;
}


BandpassFilter::BandpassFilter()
: highCutoff(0.0f)
{
}

void BandpassFilter::setupCoeffs()
{
    const float bw = (highCutoff - freq) / getSampleRate();
    const float center = (highCutoff + freq) * 0.5f;
    const float A = 2.0f * cos((center / getSampleRate()) * 6.283185307179586f);
    const float B = 1.0f - 3.0f * bw;
    const float Bsq = B * B;
    const float C = (1.0f - B * A + Bsq) / (2.0f - A);
    a[0] = 1.0f - C;
    a[1] = (C - B) * A;
    a[2] = Bsq - C;
    b[0] = B * A;
    b[1] = -Bsq;
}

tyrsound_Error BandpassFilter::setParams(tyrsound_DSPParamPair *pp)
{
    tyrsound_Error err = TYRSOUND_ERR_OK;
    for( ; pp->id; ++pp)
    {
        switch(pp->id)
        {
        case TYRSOUND_DSPPARAM_CUTOFF:
            freq = pp->val.f;
            break;

        case TYRSOUND_DSPPARAM_CUTOFF_HIGH:
            highCutoff = pp->val.f;
            break;

        default:
            tyrsound_ex_messagef(TYRSOUND_MSG_WARNING, "BandpassFilter: Unsupported param: [%u]", pp->id);
            err = TYRSOUND_ERR_UNSUPPORTED;
        }
    }
    setupCoeffs();
    return err;
}


void ResonantLowpassFilter::setupCoeffs()
{
    float f = vmin(freq, sr * 0.25f);
    R = amp;
    C = 2.0f - 2.0f * cos((f * 6.283185307179586f) / sr);
}

void ResonantLowpassFilter2::setupCoeffs()
{
    float f = vmin(freq, sr * 0.25f);
    const float fx = cos((f * 6.283185307179586f) / sr);
    const float fx1 = fx - 1.0f;
    const float t = amp * fx1;
    C = 2.0f - 2.0f * fx;
    R = (1.4142135623730951f * sqrt(-fx1 * fx1 * fx1) + t) / t;
}


#include "tyrsound_end.h"
