#ifndef TYRSOUND_PASSFILTERS_H
#define TYRSOUND_PASSFILTERS_H

#include "IIRFilter.h"

#include "tyrsound_begin.h"


class LowpassFilter : public IIRFilter<1, 4, LowpassFilter>
{
public:
    void setupCoeffs();
    tyrsound_Error setParams(tyrsound_DSPParamPair *pp);
};

class CheapLowpassFilter : public IIRFilter<1, 1, CheapLowpassFilter>
{
public:
    void setupCoeffs();
    tyrsound_Error setParams(tyrsound_DSPParamPair *pp);
};

class HighpassFilter : public IIRFilter<2, 1, HighpassFilter>
{
public:
    void setupCoeffs();
    tyrsound_Error setParams(tyrsound_DSPParamPair *pp);
};

class BandpassFilter : public IIRFilter<3, 2, BandpassFilter>
{
public:
    BandpassFilter();
    void setupCoeffs();
    tyrsound_Error setParams(tyrsound_DSPParamPair *pp);
private:
    float highCutoff;
};

class ResonantLowpassFilter : public ResonantLowpassFilterT<ResonantLowpassFilter>
{
public:
    void setupCoeffs();
};

class ResonantLowpassFilter2 : public ResonantLowpassFilterT<ResonantLowpassFilter2>
{
public:
    void setupCoeffs();
};

#include "tyrsound_end.h"


#endif
