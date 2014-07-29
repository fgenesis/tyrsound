#include "tyrsound_internal.h"
#include "ChannelGroup.h"
#include "SoundObject.h"

#include "tyrsound_begin.h"

#define LOCKED_FORALL(s, body) \
    do { MutexGuard guard(mtx); \
        for(unsigned _i = 0; _i < sounds.size(); ++_i) \
        { SoundObject *s = sounds[_i]; {body;} } \
    } while(0)


ChannelGroup *ChannelGroup::create()
{
    void *p = Alloc(sizeof(ChannelGroup));
    return new(p) ChannelGroup;
}

void ChannelGroup::destroy()
{
    this->~ChannelGroup();
    Free(this);
}

ChannelGroup::ChannelGroup() : ReferencedPlayable(TY_GROUP)
, x(0.0f), y(0.0f), z(0.0f)
, volume(1.0f)
, speed(1.0f)
{
}

ChannelGroup::~ChannelGroup()
{
    clear();
}

void ChannelGroup::clear()
{
    MutexGuard guard(mtx);
    for(unsigned i = 0; i < sounds.size(); ++i)
        _detach(sounds[i]);
    sounds.clear();
}

tyrsound_Error ChannelGroup::attachSound(SoundObject *sound)
{
    if(sound->group == this)
        return TYRSOUND_ERR_OK;
    if(sound->group)
        sound->group->detachSound(sound);

    {
        MutexGuard guard(mtx);
        if(!sounds.push_back(sound))
            return TYRSOUND_ERR_OUT_OF_MEMORY;
    }

    sound->group = this;
    sound->setGroupPosition(x, y, z);
    sound->setGroupSpeed(speed);
    sound->setGroupVolume(volume);
    return TYRSOUND_ERR_OK;
}

void ChannelGroup::detachSound(SoundObject *sound)
{
    if(sound->group == this)
        _detach(sound);
    else
        tyrsound_ex_messagef(TYRSOUND_MSG_INTERNAL_ERROR, "Sound is in wrong group (is: %p, should be: %p)", sound->group, this);
}

void ChannelGroup::_detach(SoundObject *sound)
{
    sound->group = NULL;
    sound->setGroupVolume(1);
    sound->setGroupSpeed(1);
    sound->setGroupPosition(0, 0, 0);

    MutexGuard guard(mtx);
    sounds.dropAndPop(sound);
}

tyrsound_Error ChannelGroup::setVolume(float vol)
{
    LOCKED_FORALL(s, s->setGroupVolume(vol));
    return TYRSOUND_ERR_OK;
}

tyrsound_Error ChannelGroup::setSpeed(float speed)
{
    LOCKED_FORALL(s, s->setGroupSpeed(speed));
    return TYRSOUND_ERR_OK;
}

tyrsound_Error ChannelGroup::stop()
{
    LOCKED_FORALL(s, s->stop());
    return TYRSOUND_ERR_OK;
}

tyrsound_Error ChannelGroup::play()
{
    LOCKED_FORALL(s, s->play());
    return TYRSOUND_ERR_OK;
}

tyrsound_Error ChannelGroup::pause()
{
    LOCKED_FORALL(s, s->pause());
    return TYRSOUND_ERR_OK;
}

tyrsound_Error ChannelGroup::setPosition(float x, float y, float z)
{
    LOCKED_FORALL(s, s->setGroupPosition(x, y, z));
    return TYRSOUND_ERR_OK;
}

int ChannelGroup::isPlaying()
{
    int playing = 0;
    LOCKED_FORALL(s, playing += s->isPlaying());
    return playing;
}
int ChannelGroup::isStopped()
{
    int stopped = 0;
    LOCKED_FORALL(s, stopped += s->isStopped());
    return stopped;
}



#include "tyrsound_end.h"
