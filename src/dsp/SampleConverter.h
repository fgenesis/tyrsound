#ifndef TYRSOUND_SAMPLECONVERTER_H
#define TYRSOUND_SAMPLECONVERTER_H

#include "tyrsound.h"
#include "tyrsound_begin.h"


void copyConvertToFloat(tyrsound_Format &fmt, float *dst, void *src, size_t sz);
void copyConvertToS16(tyrsound_Format &fmt, s16 *dst, void *src, size_t sz);

void convertFloatToS16Inplace(tyrsound_Format& fmt, void *buf, size_t samples);


#include "tyrsound_end.h"
#endif
