#include "tyrsound.h"
#include <stdio.h>

static int playSecs(const char *name, float secs)
{
    tyrsound_Stream strm;
    if(tyrsound_createFileNameStream(&strm, name, "rb") != TYRSOUND_ERR_OK)
    {
        printf("File not found: %s\n", name);
        return 2;
    }

    tyrsound_Handle handle = tyrsound_load(strm);
    if(handle == TYRSOUND_NULLHANDLE)
    {
        printf("Format not recognized / no suitable decoder.\n");
        return 3;
    }

    tyrsound_Error err = tyrsound_play(handle);
    if(err != TYRSOUND_ERR_OK)
    {
        printf("Failed to start playback. err = %d\n", err);
        return 4;
    }

    printf("Playing %s for %f secs ...\n", name, secs);
    const float len = tyrsound_getLength(handle);

    /* This hogs the CPU, don't do it this way in a real program :)*/
    float playpos = 0;
    while(tyrsound_isPlaying(handle) && ((playpos = tyrsound_getPlayPosition(handle)) < secs))
    {
        tyrsound_update();
        printf("[At %.3f / %.3f]\r", playpos, len);
    }

    printf("Done playing '%s'\n", name);

    /* Free resources after we're done */
    tyrsound_unload(handle);

    return 0;
}

int main(int argc, char **argv)
{
    if(tyrsound_init(NULL, NULL) != TYRSOUND_ERR_OK)
    {
        printf("Failed to init tyrsound.\n");
        return 1;
    }

    playSecs("test2.mp3", 2);
    playSecs("test.mp3", 1);
    playSecs("test.ogg", 1);
    playSecs("test2.ogg", 1);
    playSecs("test3.ogg", 1);

    tyrsound_shutdown();

	return 0;
}

