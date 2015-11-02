#ifndef TYRSOUND_DECODER_BASE_H
#define TYRSOUND_DECODER_BASE_H
#include "tyrsound_ex.h"
#include "tyrsound_begin.h"

struct DecoderConstants
{
    DecoderConstants() : length(0), totalsamples(0) {}

    float length;
    tyrsound_uint64 totalsamples; // number of samples divided by number of channels. depends on sampling rate.
};

class DecoderBase
{
protected:

    DecoderBase(const tyrsound_Format& fmt);
    virtual ~DecoderBase() {}

public:
    void destroy();

    virtual size_t fillBuffer(void *buf, size_t size) = 0;
    virtual tyrsound_Error seekSample(tyrsound_uint64 sample) = 0;
    virtual tyrsound_uint64 tellSample() = 0;
    virtual tyrsound_Error setLoop(float seconds, int loops) = 0;
    virtual float getLoopPoint() = 0;
    virtual bool isEOF() = 0;

    // have default implementations
    virtual tyrsound_Error seek(float seconds);
    virtual float tell();

    // non-virtuals
    inline float getLength() const // 0 if unknown
    {
        return c.length;
    }
    inline tyrsound_uint64 getTotalSamples() const // 0 if unknown
    {
        return c.totalsamples;
    }
    inline void getFormat(tyrsound_Format *fmt)
    {
        *fmt = _fmt;
    }

protected:

    DecoderConstants c;
    tyrsound_Format _fmt;
};

class DecoderFactoryBase
{
public:
    virtual DecoderBase *create(const tyrsound_Format& fmt, tyrsound_Stream strm) const = 0;
    virtual void staticInit() const = 0;
    virtual void staticShutdown() const = 0;
    virtual bool checkMagic(const unsigned char *magic, size_t size) const = 0;
    virtual const char *getName() const  = 0;
};

template<class T> class DecoderFactory : public DecoderFactoryBase
{
    typedef T K;
    virtual DecoderBase *create(const tyrsound_Format& fmt, tyrsound_Stream strm) const
    {
        T *decoder = K::create(fmt, strm);
        return static_cast<DecoderBase*>(decoder);
    }
    virtual void staticInit() const
    {
        K::staticInit();
    }
    virtual void staticShutdown() const
    {
        K::staticShutdown();
    }
    virtual bool checkMagic(const unsigned char *magic, size_t size) const
    {
        return K::checkMagic(magic, size);
    }
    virtual const char *getName() const
    {
        return K::getName();
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
