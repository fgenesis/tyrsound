#include "IIRFilter.h"

class LowpassFilter : public IIRFilter<1, 4>
{
public:
    LowpassFilter(float samplerate, float cutoff);

protected:
    void setupCoeffs();
};

LowpassFilter::LowpassFilter(float samplerate, float cutoff) : IIRFilter(samplerate, cutoff)
{
}

LowpassFilter::setupCoeffs()