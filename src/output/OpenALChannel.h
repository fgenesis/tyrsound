#ifndef TYRSOUND_OPENAL_CHANNEL_H
#define TYRSOUND_OPENAL_CHANNEL_H

#include "tyrChannelBase.h"

#include "tyrsound_begin.h"

class OpenALDevice;

class OpenALChannel : public ChannelBase
{
    friend class OpenALDevice;

protected:
    OpenALChannel(OpenALDevice*);
    virtual ~OpenALChannel();

private:
    // AL specific
    void setSourceid(int sid) { _sid = sid; }
    tyrsound_Error prepare();
    void setInitialALValues();
    tyrsound_Error genBuffers();

public:
    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error setPosition(float x, float y, float z);
    virtual tyrsound_Error stop();
    virtual tyrsound_Error play();
    virtual tyrsound_Error pause();
    virtual bool isPlaying();

    virtual bool wantData();
    virtual void getBuffer(void **buf, size_t *size);
    virtual void update();
    virtual tyrsound_Error filledBuffer(size_t size, const tyrsound_Format& fmt);
    virtual float getPlayPosition();

    // TODO: single buffer mode /w AL_LOOPING -- think about API

protected:
    unsigned int _sid;
    unsigned int *_bid;
    OpenALDevice *_aldev;
    char *_pcmbuf;
    size_t _pcmbufsize;
    tyrsound_int64 _samplesDone;
    bool _inUse;
    bool _paused;
    bool _playing;
    bool _initial;
    unsigned int _initialBufferIdx;
    unsigned int _numBuffers;
    float _volume;
    float _speed;
    int _hz;
};



#include "tyrsound_end.h"

#endif
