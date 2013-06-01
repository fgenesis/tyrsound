#ifndef TYRSOUND_DECODER_BASE_H
#define TYRSOUND_DECODER_BASE_H
#include "tyrsound_ex.h"
#include "tyrsound_begin.h"


class DecoderBase
{
protected:

    DecoderBase() {}
    virtual ~DecoderBase() {}

public:
    void destroy();

    virtual size_t fillBuffer(void *buf, size_t size) = 0;
    virtual float getLength() = 0;
    virtual tyrsound_Error seek(float seconds) = 0;
    virtual float tell() = 0;
    virtual tyrsound_Error setLoop(float seconds, int loops) = 0;
    virtual float getLoopPoint() = 0;
    virtual bool isEOF() = 0;
    virtual void getFormat(tyrsound_Format *fmt) = 0;
};

class DecoderFactoryBase
{
public:
    virtual DecoderBase *create(const tyrsound_Format& fmt, tyrsound_Stream strm) = 0;
};

template<class T> class DecoderFactory : public DecoderFactoryBase
{
    typedef typename T K;
    virtual DecoderBase *create(const tyrsound_Format& fmt, tyrsound_Stream strm)
    {
        T *decoder = K::create(fmt, strm);
        return static_cast<DecoderBase*>(decoder);
    }
};

template<typename T> struct DecoderRegistrar
{
    DecoderRegistrar()
    {
        static DecoderFactory<T> instance;
        tyrsound_ex_registerDecoder(&instance);
    }
};

#define TYRSOUND_REGISTER_DECODER(type) \
    TYRSOUND_STATIC_REGISTER(DecoderRegistrar, type, )


#include "tyrsound_end.h"
#endif
