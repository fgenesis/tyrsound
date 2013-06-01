#ifndef TYRSOUND_SOUNDOBJECT_H
#define TYRSOUND_SOUNDOBJECT_H
#include "tyrsound_begin.h"

class DecoderBase;
class ChannelBase;

class SoundObject
{
protected:
    SoundObject(DecoderBase *decoder, ChannelBase *output);
    virtual ~SoundObject();

public:
    static SoundObject *create(DecoderBase *decoder, ChannelBase *output);
    void destroy();
    void update();

    float getPlayPosition();
    tyrsound_Error getPosition(float *x, float *y, float *z);
    tyrsound_Error setPosition(float x, float y, float z);
    //float getVolume() const; // TODO
    tyrsound_Error setVolume(float vol);
    tyrsound_Error setSpeed(float speed);
    tyrsound_Error stop();
    tyrsound_Error pause();
    tyrsound_Error play();
    bool isPlaying();

    // sets decoder position. Has no immediate effect on the already buffered data!
    tyrsound_Error seek(float seconds);
    float tell();
    tyrsound_Error setLoop(float seconds, int loops);
    float getLength();

    unsigned int _idxInStore;

protected:
    DecoderBase *_decoder;
    ChannelBase *_channel;
};

#include "tyrsound_end.h"
#endif