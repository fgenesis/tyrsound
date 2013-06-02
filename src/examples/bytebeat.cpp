/* Bytebeat generator, as example for a custom "decoder".
 * See http://pouet.net/topic.php?which=8357 for how it all started
 * See http://pelulamu.net/countercomplex/music_formula_collection.txt for a collection of formulas
*/

#include <stdio.h>
#include <string.h>
#include <new>
#include <math.h>

#include "tyrDecoderBase.h"


class SoundGenerator : public tyrsound::DecoderBase
{
protected:

    SoundGenerator() : _t(0) {}
    virtual ~SoundGenerator() {}

    unsigned int _t;

    inline static char getByte(unsigned int t)
    {
        // by mu6k
        int x, y;
        return char((((int)(3e3/(y=t&16383))&1)*35) +
        (x=t*("6689"[t>>16&3]&15)/24&127)*y/4e4 +
        ((t>>8^t>>10|t>>14|x)&63));
    }

public:

    static SoundGenerator *create(const tyrsound_Format& fmt, tyrsound_Stream strm)
    {
        char start[4];
        strm.read(start, 4, 1, strm.user);
        if(memcmp(start, "BYB0", 4)) // Is the incoming stream meant to be "played" by this decoder?
            return NULL;

        void *mem = tyrsound::Alloc(sizeof(SoundGenerator));
        if(!mem)
            return NULL;

        SoundGenerator *g = new(mem) SoundGenerator();
        return g;
    }

    virtual size_t fillBuffer(void *buf, size_t size)
    {
        if(isEOF())
            return 0;

        char *out = (char*)buf;
        // This generator produces 8 bit mono samples.
        // If using more channels, divide size by number of channels.
        const unsigned int tend = _t + size; 
        for(unsigned int t = _t; t < tend; ++t)
        {
            *out++ = getByte(t);
        }
        _t += size;
        return size;
    }
    virtual float getLength() { return -1.0f; }
    virtual tyrsound_Error seek(float seconds) { return TYRSOUND_ERR_UNSUPPORTED; }
    virtual float tell() { return -1.0f; }
    virtual tyrsound_Error setLoop(float seconds, int loops) { return TYRSOUND_ERR_UNSUPPORTED; }
    virtual float getLoopPoint() { return -1.0f; }
    virtual bool isEOF() { return _t > 3000000; } // arbitrarily high sample count to stop at some point
    virtual void getFormat(tyrsound_Format *fmt)
    {
        fmt->sampleBits = 8;
        fmt->hz = 32000;
        fmt->channels = 1;
        fmt->signedSamples = 0;
    }
};

TYRSOUND_REGISTER_DECODER(SoundGenerator);


int main(int argc, char **argv)
{
    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        return 1;

    tyrsound_Stream strm;
    // This creates a 4-byte identifier to allow selecting the decoder
    tyrsound_createMemStream(&strm, (void*)"BYB0", 4, NULL);

    tyrsound_Handle handle = tyrsound_load(strm, NULL);
    if(handle == TYRSOUND_NULLHANDLE)
        return 2;

    tyrsound_play(handle);

    while(tyrsound_isPlaying(handle))
        tyrsound_update();

    tyrsound_unload(handle);

    tyrsound_shutdown();

    return 0;
}

