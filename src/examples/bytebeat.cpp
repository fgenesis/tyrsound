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
    tyrsound_Format _fmt;

    char (*genfunc)(unsigned int);


    // by bst
    static char getByte1(unsigned int t)
    {
        return (int)(t/1e7*t*t+t)%127|t>>4|t>>5|t%127+(t>>16)|t;
    }

    // by ryg
    static char getByte2(unsigned int t)
    {
        return ((t*("36364689"[t>>13&7]&15))/12&128)
        +(((((t>>12)^(t>>12)-2)%11*t)/4|t>>13)&127);
    }

    // by mu6k
    static char getByte3(unsigned int t)
    {
        int x, y;
        return char((((int)(3e3/(y=t&16383))&1)*35) +
        (x=t*("6689"[t>>16&3]&15)/24&127)*y/4e4 +
        ((t>>8^t>>10|t>>14|x)&63));
    }

public:


    static SoundGenerator *SoundGenerator::create(const tyrsound_Format& fmt, tyrsound_Stream strm)
    {
        char start[4];
        strm.read(start, 4, 1, strm.user);
        if(memcmp(start, "BYB", 3))
            return NULL;
        int which = start[3] - '0';

        tyrsound_Format f;
        memset(&f, 0, sizeof(f));
        f.sampleBits = 8;
        f.hz = 8000;
        f.channels = 1;
        f.signedSamples = 0;
        if(which > 2)
            return NULL;

        void *mem = tyrsound::Alloc(sizeof(SoundGenerator));
        if(!mem)
            return NULL;

        SoundGenerator *g = new(mem) SoundGenerator();

        switch(which)
        {
        case 1:
            g->genfunc = getByte1;
            break;
        case 2:
            f.hz = 44100;
            g->genfunc = getByte2;
            break;
        case 3:
            g->genfunc = getByte3;
            f.hz = 32000;
            break;
        default:
            return NULL;
        }

        g->_fmt = f;
        return g;
    }

    virtual size_t fillBuffer(void *buf, size_t size)
    {
        if(isEOF())
            return 0;

        char *out = (char*)buf;
        const unsigned int tend = _t + size;
        for(unsigned int t = _t; t < tend; ++t)
        {
            *out++ = genfunc(t);
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
        *fmt = _fmt;
    }
};

TYRSOUND_REGISTER_DECODER(SoundGenerator);


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage: './%s ID' -- where ID is either 1, 2, or 3.\n", argv[0]);
        return 0;
    }

    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
        return 1;

    char ident[4] = {'B', 'Y', 'B', '0' + (char)atoi(argv[1]) };

    tyrsound_Stream strm;
    tyrsound_createMemStream(&strm, &ident[0], sizeof(ident), NULL);

    tyrsound_Handle handle = tyrsound_load(strm, NULL);
    if(handle == TYRSOUND_NULLHANDLE)
        return 2;

    tyrsound_play(handle);

    while(tyrsound_isPlaying(handle))
        tyrsound_update();

    tyrsound_shutdown();

    return 0;
}

