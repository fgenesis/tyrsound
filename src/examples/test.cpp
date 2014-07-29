#include "tyrsound.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

static int playSecs(const char *name, float secs)
{
    tyrsound_Stream strm;
    if(tyrsound_createFileNameStream(&strm, name, "rb") != TYRSOUND_ERR_OK)
    {
        printf("File not found: %s\n", name);
        return 2;
    }

    tyrsound_Sound sound = tyrsound_load(&strm);
    if(sound == TYRSOUND_NULL_SOUND)
    {
        printf("Format not recognized / no suitable decoder.\n");
        return 3;
    }

    tyrsound_Error err = tyrsound_play(sound);
    if(err != TYRSOUND_ERR_OK)
    {
        printf("Failed to start playback. err = %d\n", err);
        return 4;
    }

    printf("Playing %s for %f secs ...\n", name, secs);
    const float len = tyrsound_getLength(sound);

    /* This hogs the CPU, don't do it this way in a real program :)*/
    float playpos = 0;
    while(tyrsound_isPlaying(sound) && ((playpos = tyrsound_getPlayPosition(sound)) < secs))
    {
#ifdef _WIN32
        Sleep(100);
#endif
        tyrsound_update();
        //printf("[At %.3f / %.3f]\r", playpos, len);
    }

    printf("Done playing '%s'\n", name);

    /* Free resources after we're done */
    tyrsound_unload(sound);

    return 0;
}

int main(int argc, char **argv)
{
    tyrsound_Format fmt;
    memset(&fmt, 0, sizeof(fmt));
    tyrsound_DeviceConfig cfg;
    memset(&cfg, 0, sizeof(cfg));

    fmt.sampleBits = 16;
    fmt.channels = 0;
    fmt.hz = 44100;
    fmt.bigendian = 0;
    fmt.signedSamples = 1;
    fmt.isfloat = 1;

    cfg.bufferSize = 16 * 1024 + 1 * 1024; // use some weird buffer size
    cfg.numBuffers = 16;

    if(tyrsound_init(&fmt, &cfg) != TYRSOUND_ERR_OK)
    {
        printf("Failed to init tyrsound.\n");
        return 1;
    }

    tyrsound_Stream bgstrm;
    tyrsound_createFileNameStream(&bgstrm, "test.mod", "rb");
    tyrsound_Sound sound = tyrsound_load(&bgstrm);
    tyrsound_fireAndForget(sound);

    tyrsound_Group g = tyrsound_createGroup();
    tyrsound_setGroup(sound, g);
    tyrsound_setSpeed(g, 2);
    tyrsound_unload((tyrsound_Sound)g); // should detect wrong handle type even if doing stupid things

    playSecs("test.s3m", 2);
    tyrsound_play(sound);
    playSecs("test.wav", 2);
    playSecs("test.flac", 2);
    playSecs("test2.mp3", 2);
    playSecs("test.mp3", 1);
    playSecs("test.ogg", 1);
    tyrsound_stop(sound);
    playSecs("test2.ogg", 1);
    playSecs("test3.ogg", 1);

    tyrsound_unload(sound); // handle is invalid here because autodeleted

    tyrsound_shutdown();

    return 0;
}

