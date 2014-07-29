#ifndef TYRSOUND_SOUNDOBJECT_H
#define TYRSOUND_SOUNDOBJECT_H

#include "BasicClasses.h"

#include "tyrsound_begin.h"

class ChannelGroup;
class DecoderBase;
class ChannelBase;

class SoundObject : public ReferencedPlayable
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
    void detachFromGroup();

    float getPlayPosition();
    tyrsound_Error getPosition(float *x, float *y, float *z);
    tyrsound_Error setPosition(float x, float y, float z);
    //float getVolume() const; // TODO
    tyrsound_Error setVolume(float vol);
    tyrsound_Error setSpeed(float speed);
    tyrsound_Error stop();
    tyrsound_Error pause();
    tyrsound_Error play();
    int isPlaying();
    int isStopped();

    // sets decoder position. Has no immediate effect on the already buffered data!
    tyrsound_Error seek(float seconds);
    float tell();
    tyrsound_Error setLoop(float seconds, int loops);
    float getLength();

    tyrsound_Error updateVolume();
    tyrsound_Error updatePosition();
    tyrsound_Error updateSpeed();

protected:
    void _decode();

    DecoderBase *_decoder;
    ChannelBase *_channel;
    bool _update;
    bool _autofree;
    bool _initial;
    
    float _volume;
    float _speed;
    float _posX, _posY, _posZ;

public:

    ChannelGroup *group;

    tyrsound_Error setGroupPosition(float x, float y, float z);
    tyrsound_Error setGroupVolume(float vol);
    tyrsound_Error setGroupSpeed(float speed);

protected:
    float groupvolume;
    float groupspeed;
    float groupx, groupy, groupz;

};

#include "tyrsound_end.h"
#endif