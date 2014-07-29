#ifndef TYRSOUND_SOUNDOBJECT_H
#define TYRSOUND_SOUNDOBJECT_H

#include "BasicClasses.h"

#include "tyrsound_begin.h"


class DecoderBase;
class ChannelBase;

class SoundObject : public Referenced, public Playable
{
protected:
    SoundObject(DecoderBase *decoder);
    virtual ~SoundObject();

public:
    static SoundObject *create(DecoderBase *decoder);
    void destroy();
    void update();
    inline void setAutoFree(bool on) { _autofree = on; }
    inline bool isAutoFree() const { return _autofree; }

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
    bool isStopped();

    // sets decoder position. Has no immediate effect on the already buffered data!
    tyrsound_Error seek(float seconds);
    float tell();
    tyrsound_Error setLoop(float seconds, int loops);
    float getLength();

protected:
    void _decode();

    DecoderBase *_decoder;
    ChannelBase *_channel;
    bool _update;
    bool _autofree;
    
    float _volume;
    float _speed;
    float _posX, _posY, _posZ;

};

#include "tyrsound_end.h"
#endif