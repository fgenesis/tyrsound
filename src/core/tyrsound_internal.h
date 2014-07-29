#ifndef TYRSOUND_INTERNAL_H
#define TYRSOUND_INTERNAL_H

#include <stdlib.h>
#include <string.h>
#include <new>

#include "tyrsound_ex.h"

#include "tyrsound_begin.h"

enum Type
{
    TY_SOUND,
    TY_GROUP,
    TY_EFFECT
}; // type id must not exceed 7 (see HandleBits)

// Definitions
class SoundObject;
class ObjectStore;
class ChannelGroup;


// in tyrsound_device.cpp
DeviceBase *getDevice();
bool initDevice(const char *name, const tyrsound_Format *fmt, const tyrsound_DeviceConfig *cfg);
void shutdownDevice();

// in tyrsound_sound.cpp
tyrsound_Sound registerSoundObject(SoundObject *);
void registerUpdate(SoundObject *);
void unregisterUpdate(SoundObject *);
tyrsound_Error initSounds();
void shutdownSounds();
tyrsound_Error updateSounds();

// in tyrsound_group.cpp
tyrsound_Error initGroups();
void shutdownGroups();


// in tyrsound_load.cpp
void initDecoders();
void shutdownDecoders();

// in tyrsound_misc.cpp
void breakpoint();
bool isBigEndian();
unsigned int readLE32(const void *buf);
unsigned short readLE16(const void *buf);
void writeLE32(void *buf, unsigned int i);
void writeLE16(void *buf, unsigned short i);

// in tyrsound_dyn.cpp
void *dynopen(const char *fn);
void dynclose(void *);
void *dynsym(void *, const char *name);

// Helper for static initialization
template <typename T, int SZ> class RegistrationHolder
{
public:
    RegistrationHolder() : _idx(0) {}
    static void Register(const T& item) { instance().add(item); }
    static const T& Get(size_t i) { return instance()._data[i]; }
    static size_t Size() { return instance()._idx; }

private:
    static RegistrationHolder<T, SZ>& instance()
    {
        static RegistrationHolder<T, SZ> holder;
        return holder;
    }
    void add(const T& item) { _data[_idx++] = item; }
    T _data[SZ];
    size_t _idx;
};

template<typename T> void tyrsound_T_streamsToInterleaved(T *out, T * const * const channels, size_t nsamples, unsigned nchan)
{
    switch(nchan)
    {
        case 1:
            memcpy(out, channels[0], sizeof(T) * nsamples);
            break;
        case 2:
        {
            const T * const ch0 = channels[0];
            const T * const ch1 = channels[1];
            for (size_t i = 0; i < nsamples; i++)
            {
                *out++ = ch0[i];
                *out++ = ch1[i];
            }
            break;
        }
        case 3:
        {
            const T * const ch0 = channels[0];
            const T * const ch1 = channels[1];
            const T * const ch2 = channels[2];
            for (size_t i = 0; i < nsamples; i++)
            {
                *out++ = ch0[i];
                *out++ = ch1[i];
                *out++ = ch2[i];
            }
            break;
        }
        case 4:
        {
            const T * const ch0 = channels[0];
            const T * const ch1 = channels[1];
            const T * const ch2 = channels[2];
            const T * const ch3 = channels[3];
            for (size_t i = 0; i < nsamples; i++)
            {
                *out++ = ch0[i];
                *out++ = ch1[i];
                *out++ = ch2[i];
                *out++ = ch3[i];
            }
            break;
        }
        case 5:
        {
            const T * const ch0 = channels[0];
            const T * const ch1 = channels[1];
            const T * const ch2 = channels[2];
            const T * const ch3 = channels[3];
            const T * const ch4 = channels[4];
            for (size_t i = 0; i < nsamples; i++)
            {
                *out++ = ch0[i];
                *out++ = ch1[i];
                *out++ = ch2[i];
                *out++ = ch3[i];
                *out++ = ch4[i];
            }
            break;
        }
        default:
            for (size_t i = 0; i < nsamples; i++)
                for(unsigned c = 0; c < nchan; ++c)
                    *out++ = channels[c][i];
            break;
    }
}

extern ObjectStore soundstore;
extern ObjectStore groupstore;


#include "tyrsound_end.h"
#endif

