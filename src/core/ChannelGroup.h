#ifndef TYRSOUND_CHANNELGROUP_H
#define TYRSOUND_CHANNELGROUP_H

#include "BasicClasses.h"

#include "tyrsound_begin.h"

class ChannelGroup : public ReferencedPlayable
{
public:
    ChannelGroup();
    virtual ~ChannelGroup();
    static ChannelGroup *create();
    void destroy();

    void clear();
    tyrsound_Error attachSound(SoundObject *sound);
    void detachSound(SoundObject *sound);

    tyrsound_Error setVolume(float vol);
    tyrsound_Error setSpeed(float speed);
    tyrsound_Error stop();
    tyrsound_Error play();
    tyrsound_Error pause();
    tyrsound_Error setPosition(float x, float y, float z);
    int isPlaying();
    int isStopped();

protected:
    void _detach(SoundObject *sound);

    PODArray<SoundObject*> sounds;
    float x, y, z;
    float volume;
    float speed;
    Mutex mtx;
};

#include "tyrsound_end.h"

#endif
